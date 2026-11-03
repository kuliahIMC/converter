AUDIT TAHAP 2 — Race Condition & Batch Safety (terbukti dari source aktual)
Nama: Daiyana | Tidak ada file diubah | Tidak ada build/refactor

=== 1. RACE CONDITION ANTARA QTimer::singleShot, cancelConversion, finishBatchConversion ===
TERBUKTI.
- photo_page.cpp:437-448: QTimer::singleShot(0) menjadwalkan lambda yang memanggil converter_->convert().
- photo_page.cpp:451-470: cancelConversion() memanggil converter_->cancel() yang langsung kill QProcess dan emit cancelled().
- photo_page.cpp:42-48: signal cancelled terhubung ke lambda yang memanggil finishBatchConversion().
- photo_page.cpp:414-419: processNextFile() juga bisa memanggil finishBatchConversion() jika batchCancelled_ atau pendingFiles_ kosong.
SKENARIO PEMICU: cancelConversion dipanggil tepat setelah QTimer::singleShot(0) tapi sebelum converter_->convert(). Lambda di QTimer akan tetap berjalan (singleShot sudah dijadwalkan). Lambda memeriksa batchCancelled_ (line 438) — ini mengurangi risiko, tapi tidak mencegah race jika cancel terjadi antara pengecekan batchCancelled_ dan pemanggilan convert().

=== 2. finishBatchConversion DIPANGGIL LEBIH DARI SEKALI ===
TERBUKTI.
- photo_page.cpp:472: tidak ada guard (misal bool finished_) untuk mencegah pemanggilan berulang.
SKENARIO PEMICU: batchCancelled_ = true; cancelConversion() dipanggil saat pendingFiles_ kosong → finishBatchConversion() dipanggil (line 456). Kemudian signal cancelled dari converter juga memicu finishBatchConversion() (line 46) → dua kali panggilan, dua QMessageBox.

=== 3. QMessageBox MUNCUL DUA KALI ===
TERBUKTI (konsekuensi poin 2).
- photo_page.cpp:511-515: QMessageBox::information dipanggil setiap kali finishBatchConversion() dijalankan. Tanpa guard, dua panggilan = dua kotak pesan.

=== 4. QPROCESS BERHENTI SEBELUM TEMPORARY DIRECTORY DIBERSIHKAN ===
RISIKO TERBUKTI, BUKAN JENIS BUG OTOMATIS.
- image_converter_service.cpp:50-57: cleanup() menghapus temporaryDir_ (line 52) dan menyetel running_ = false, tapi tidak menunggu process_->waitForFinished().
- image_converter_service.cpp:34-43: destructor memanggil kill() lalu waitForFinished(1000), tapi cleanup() tidak dipanggil di dalam handleFinished sebelum emit finished — cleanup dipanggil setelah emit (line 328).
SKENARIO PEMICU: jika QProcess belum benar-benar berhenti saat cleanup() menghapus temporaryDir_, file sementara (mis. input.png) bisa masih terbuka oleh proses anak, menyebabkan penghapusan gagal atau file terkunci. Ini belum terbukti otomatis tanpa menjalankan proses, tapi kode tidak menjamin urutan aman.

=== 5. OUTPUT BISA DIBACA KEMBALI ===
TERBUKTI SECARA SINTAKS, TAPI BELUM TERUJI.
- image_converter_service.cpp:314-317: outputExists dicek (file ada dan size > 0).
- image_converter_service.cpp:330: emit finished(result) mengirim outputPath_.
- photo_page.cpp:472-516: finishBatchConversion() hanya menampilkan QMessageBox, TIDAK membaca kembali file output untuk verifikasi integritas (tidak ada pembacaan ulang gambar, checksum, atau QImageReader terhadap output).
Jadi: file output bisa dibaca oleh aplikasi eksternal, tapi aplikasi ini sendiri tidak memverifikasinya.

=== BUG NYATA vs RISIKO BELUM TERBUKTI ===
NYATA (terbukti dari kode):
- finishBatchConversion bisa dipanggil ganda (tidak ada guard).
- QMessageBox bisa muncul ganda (konsekuensi di atas).
- Race antara QTimer singleShot dan cancel (mitigasi parsial lewat batchCancelled_ tapi tidak lengkap).

RISIKO (kode mendukung, tapi memerlukan eksekusi nyata untuk konfirmasi):
- Temporary directory dibersihkan sebelum QProcess benar-benar selesai (urutan kode tidak menjamin).
- Output tidak diverifikasi kembali setelah konversi.
- Batch tidak memiliki mekanisme rollback jika satu file gagal di tengah batch.

=== REKOMENDASI MINIMAL (tanpa refactor besar) ===
- Tambahkan bool finishedBatch_ = false; di finishBatchConversion() return jika sudah true, set true setelah QMessageBox.
- Tambahkan guard di processNextFile() dan cancelConversion(): jika pendingFiles_ kosong DAN converter_->isRunning() false, jangan panggil finishBatchConversion() lagi.
- Di cleanup(): tambahkan process_->waitForFinished(500) sebelum delete temporaryDir_ (jika running masih true).
- Untuk verifikasi output: setelah emit finished, tambahkan QImageReader(outputPath_) untuk memastikan file dapat dibaca kembali.
