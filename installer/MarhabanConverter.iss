
#define MyAppName "Marhaban Converter"
#define MyAppVersion "2.1.0"
#define MyAppPublisher "Marhaban Studio"
#define MyAppURL "https://marhabanstudio.my.id"
#define MyAppExeName "MarhabanConverter.exe"

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

VersionInfoVersion=2.1.0.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Installer
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

DefaultDirName={autopf}\Marhaban Studio\Marhaban Converter
DefaultGroupName=Marhaban Studio\Marhaban Converter
UsePreviousAppDir=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

OutputDir=output
OutputBaseFilename=Marhaban-Converter-Setup-v{#MyAppVersion}

CloseApplications=yes
RestartApplications=no
Uninstallable=yes

Compression=lzma2
SolidCompression=yes
WizardStyle=modern

SetupIconFile=..\assets\Marhaban-studio.ico
UninstallDisplayName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel1=Selamat datang di {#MyAppName}
WelcomeLabel2=Wizard ini akan memasang [name/ver] di komputer kamu.%n%nKonversi audio, video, foto, dan kompres media dalam satu aplikasi.
FinishedHeadingLabel=Instalasi selesai
FinishedLabel={#MyAppName} berhasil dipasang.%n%nKlik Selesai untuk menutup wizard.

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
Filename: "{app}\{#MyAppExeName}"; Description: "Jalankan {#MyAppName}"; WorkingDir: "{app}"; Flags: postinstall nowait skipifsilent
