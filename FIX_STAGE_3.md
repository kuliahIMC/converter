FIX STAGE 3 — Photo Converter (terbukti dari source aktual)
Nama: Daiyana | Tidak ada refactor arsitektur | Build berhasil

=== MASALAH DIPERBAIKI ===
1. RACE CONDITION QTimer (photo_page.cpp):
   - Tambah batchFinished_ guard di finishBatchConversion() (line 472).
   - Reset batchFinished_ = false di startBatchConversion() (line 399).
2. DOUBLE finishBatchConversion (photo_page.cpp):
   - Guard: return jika batchFinished_ sudah true (line 474-475).
   - Ini mencegah QMessageBox ganda.
3. QProcess + TEMPORARY DIRECTORY (image_converter_service.cpp):
   - cleanup() sekarang memanggil process_->waitForFinished(1000) sebelum delete temporaryDir_ (line 53-55).
4. OUTPUT VALIDATION (image_converter_service.cpp):
   - handleFinished(): setelah outputExists, baca ulang dengan QImageReader (line 329-336).
   - Jika gagal dibaca, hapus file dan emit failed.

=== PERUBAHAN PER FILE ===
- src/ui/photo_page.h: tambah bool batchFinished_ = false
- src/ui/photo_page.cpp: guard finishBatchConversion, reset batchFinished_ di startBatchConversion
- src/services/image_converter_service.cpp: waitForFinished di cleanup, QImageReader validasi di handleFinished

=== HASIL BUILD ===
Build berhasil (cmake --build build): MarhabanConverter.exe terhubung, tidak ada error kompilasi.

=== RISIKO MASIH TERSISA ===
- QProcess kill() tidak menjamin proses anak (ffmpeg) berhenti instan; waitForFinished(1000) hanya timeout, bukan jaminan.
- Batch masih serial; jika file besar, batch bisa lambat.
- Format DDS belum diuji runtime; SVG masih menggunakan Qt render.
- Tidak ada pengujian otomatis (tests/ kosong).
- Output validation hanya membaca frame pertama; tidak memverifikasi semua frame (meski target hanya 1 frame).

=== PENGUJIAN MANUAL YANG HARUS DILAKUKAN ===
- Konversi JPG/PNG/WEBP/BMP/TIFF/AVIF/JP2/TGA/ICO ke semua format output.
- Batch 3-5 file: mulai, cancel saat proses berjalan, pastikan QMessageBox hanya satu.
- Batch cancel saat belum mulai: pastikan tidak ada crash.
- Verifikasi file output dapat dibuka di image viewer.
- Verifikasi temporary dir bersih setelah aplikasi keluar.
- Uji dengan file input yang rusak/kosong: harus menghasilkan error yang benar.

CATATAN: HANYA BUILD YANG TERBUKTI. RUNTIME BELUM DIUJI.
