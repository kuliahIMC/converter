; ============================================================
;  Marhaban Converter - Inno Setup script
;  Letakkan di: installer\MarhabanConverter.iss
;  Opsional (kalau ada, otomatis dipakai):
;    installer\assets\icon.ico          -> ikon installer & uninstaller
;    installer\assets\wizard-large.bmp  -> gambar sisi kiri wizard (164x314)
;    installer\assets\wizard-small.bmp  -> logo kecil kanan atas (55x55)
;    installer\assets\license.txt       -> halaman lisensi
; ============================================================

#define MyAppName      "Marhaban Converter"
#define MyAppVersion   "2.0.0"
#define MyAppPublisher "Marhaban Studio"
#define MyAppURL       "https://marhabanstudio.my.id"
#define MyAppExeName   "MarhabanConverter.exe"

[Setup]
AppId={{8C9A1F72-5B41-4D3A-9E71-2F6B8C4D1020}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppCopyright=Copyright (C) 2026 {#MyAppPublisher}

; Info versi yang tampil di Properties > Details pada file Setup.exe
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Setup
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

DefaultDirName={autopf}\Marhaban Studio\Marhaban Converter
DefaultGroupName=Marhaban Studio\Marhaban Converter
DisableProgramGroupPage=yes
DisableWelcomePage=no
UsePreviousAppDir=yes

OutputDir=output
OutputBaseFilename=Marhaban-Converter-Setup-v{#MyAppVersion}

UninstallDisplayName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}

; Kalau Inno Setup kamu versi lama (di bawah 6.3) ganti ke: x64
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin

; Tutup aplikasi yang sedang berjalan saat update, tanpa restart otomatis
CloseApplications=yes
RestartApplications=no

Compression=lzma2/ultra64
SolidCompression=yes
LZMAUseSeparateProcess=yes

WizardStyle=modern
WizardResizable=no

#ifexist "..\assets\Marhaban-studio.ico"
SetupIconFile=..\assets\Marhaban-studio.ico
#endif
#ifexist "assets\wizard-large.bmp"
WizardImageFile=assets\wizard-large.bmp
#endif
#ifexist "assets\wizard-small.bmp"
WizardSmallImageFile=assets\wizard-small.bmp
#endif
#ifexist "assets\license.txt"
LicenseFile=assets\license.txt
#endif

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel1=Selamat datang di {#MyAppName}
WelcomeLabel2=Wizard ini akan memasang [name/ver] di komputer kamu.%n%nSemua alat konversi audio, video, foto, dan kompres media dalam satu aplikasi.%n%nTutup aplikasi lain sebelum melanjutkan.
FinishedHeadingLabel=Instalasi selesai
FinishedLabel={#MyAppName} sudah terpasang di komputer kamu.%n%nKlik Finish untuk menutup wizard.

[CustomMessages]
LaunchProgram=Jalankan {#MyAppName}

[Tasks]
Name: "desktopicon"; Description: "Buat shortcut di Desktop"; GroupDescription: "Shortcut tambahan:"; Flags: unchecked

[Files]
Source: "payload\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\Website Marhaban Studio"; Filename: "{#MyAppURL}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram}"; WorkingDir: "{app}"; Flags: postinstall nowait skipifsilent

[UninstallDelete]
; Bersihkan sisa file yang dibuat aplikasi saat berjalan (log, cache)
Type: filesandordirs; Name: "{app}\logs"
Type: filesandordirs; Name: "{app}\cache"
