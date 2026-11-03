LAPORAN AUDIT PHOTO CONVERTER — Marhaban App
Nama: Daiyana
Tanggal: 2026-11-02
Status: INSPEKSI SAJA (tidak ada perubahan source, build, atau file)

=== RINGKASAN ===
Fitur Photo Converter (ImageConverterService + PhotoPage) belum siap pre-release. Terdapat bug kritis pada kode konversi (hardcoded format output, format SVG tidak ditangani), tidak ada pengujian otomatis, tidak ada penanganan transparansi/EXIF/orientasi, tidak ada validasi memori atau race condition, dan QProcess tidak memiliki timeout atau penanganan korup file.

=== TEMUAN BERDASARKAN TINGKAT KEPARAHAN ===

KRITIS:
- image_converter_service.cpp:108: QImageWriter(writer) menggunakan hardcoded "ico" untuk semua format (bukan format dari parameter).
- image_converter_service.cpp:199-206: SVG disebut perlu dirender, tapi hanya SVG dan ICO yang diproses di prepareQtInput; format lain langsung lewat.
- image_converter_service.cpp:261: arguments << outputPath; tanpa validasi file korup atau pengecekan file output lengkap.
- photo_page.cpp:449: QTimer::singleShot(0) memicu batch tanpa mekanisme sinkronisasi aman terhadap race condition saat cancel.
- Tidak ada pengujian unit/integrasi di folder tests/ (kosong).

TINGGI:
- Tidak ada penanganan EXIF atau orientasi gambar secara eksplisit (setAutoTransform hanya untuk rotasi dasar, bukan EXIF Orientation tag lengkap).
- Tidak ada validasi memori atau deteksi memory leak saat batch besar.
- cancelConversion() (photo_page.cpp:451): ketika proses belum berjalan, pendingFiles_ dihapus tapi batchCancelled_ disetel, kemudian finishBatchConversion dipanggil tanpa penanganan file output parsial.
- Tidak ada mekanisme verifikasi integritas output (checksum/hash) terhadap file korup.

MENENGAH:
- Transparansi (alpha channel) tidak divalidasi saat konversi ke format yang tidak mendukung alpha (misalnya JPG).
- QTemporaryDir digunakan tapi cleanup() menghapus pointer tanpa memastikan proses QProcess selesai sepenuhnya; potensi race saat destructor.
- Format input JPG, PNG, WEBP, BMP, TIFF, AVIF, JP2, TGA, DDS, ICO, SVG terdaftar di FormatRegistry, tapi penanganan DDS tidak eksplisit di ImageConverterService.
- Performa batch: tidak ada throttling atau batasan jumlah file paralel; QProcess satu per konversi berarti serial, tapi tanpa mekanisme backpressure.

RENDAH:
- Deployment Qt/FFmpeg: CMakeLists.txt hanya menghubungkan Qt6::Widgets; tidak ada konfigurasi deployment FFmpeg dalam installer (Inno Setup) yang terverifikasi.
- Temporary directory: QTemporaryDir otomatis, tapi tidak ada batasan ukuran atau pembersihan eksplisit jika aplikasi crash.
- Tidak ada logging error detail ke file; hanya emit signal failed.

=== PENGUJIAN YANG BELUM DILAKUKAN ===
- Konversi aktual semua format input (JPG, PNG, WEBP, BMP, TIFF, AVIF, JP2, TGA, DDS, ICO, SVG) ke semua format output.
- Batch conversion multi-file dengan verifikasi output tidak korup.
- Cancel conversion saat QProcess sedang berjalan dan saat belum berjalan.
- Verifikasi transparansi tetap atau hilang sesuai format output.
- Validasi orientasi EXIF (gambar dengan tag rotasi 90/180/270 derajat).
- Memory profiling saat batch besar (>100 file).
- Race condition antara cancel() dan handleFinished().
- File korup input: apakah ditolak atau menghasilkan output salah.
- Deployment: apakah FFmpeg dan Qt DLL terbundel dengan benar.

=== KESIMPULAN KESIAPAN PRE-RELEASE ===
Fitur Photo Converter TIDAK LULUS pre-release. Alasan utama: kode konversi memiliki bug logika kritis (format output salah), tidak ada pengujian otomatis, tidak ada penanganan transparansi/EXIF, tidak ada verifikasi file korup, dan mekanisme cancel serta batch belum teruji terhadap race condition. Rekomendasi: perbaiki bug kode, tambahkan pengujian, verifikasi semua format input/output, uji cancel dan batch, lalu audit ulang sebelum release.
