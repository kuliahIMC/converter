
<p align="center">
  <img src="asset/Marhaban-studio.ico" width="96" alt="Marhaban Studio">
</p>

<h1 align="center">Marhaban Converter</h1>

<p align="center">
  <strong>Modern Desktop Media Converter for Windows</strong>
</p>

<p align="center">
  Convert, compress, and manage your media files through a modern native desktop application.
</p>

<p align="center">
  <a href="https://github.com/kuliahIMC/marhaban-converter/releases">
    <img src="https://img.shields.io/github/v/release/kuliahIMC/marhaban-converter?style=for-the-badge" alt="Latest Release">
  </a>
  <a href="https://github.com/kuliahIMC/marhaban-converter/releases">
    <img src="https://img.shields.io/github/downloads/kuliahIMC/marhaban-converter/total?style=for-the-badge" alt="Downloads">
  </a>
  <img src="https://img.shields.io/badge/Windows-x64-0078D4?style=for-the-badge" alt="Windows x64">
  <img src="https://img.shields.io/badge/C++-17-00599C?style=for-the-badge" alt="C++17">
  <img src="https://img.shields.io/badge/Qt-6.12-41CD52?style=for-the-badge" alt="Qt 6">
  <img src="https://img.shields.io/badge/FFmpeg-9.0.1-007808?style=for-the-badge" alt="FFmpeg">
  <img src="https://img.shields.io/badge/License-Proprietary-6F4BFF?style=for-the-badge" alt="Proprietary License">
</p>

---

## ✨ Overview

**Marhaban Converter** is a native Windows desktop application developed by **Marhaban Studio** for converting and processing images, videos, and audio files.

Built with C++ and Qt 6, the application focuses on delivering a modern desktop experience with a clean interface, modular architecture, and efficient media processing capabilities.

Marhaban Converter combines:

- **C++17** for application logic and core functionality.
- **Qt 6** for the native desktop interface.
- **FFmpeg** for audio and video processing.
- **FFprobe** for media inspection and metadata extraction.
- **CMake** for project configuration and build management.
- **MinGW** for Windows compilation.
- **Inno Setup** for Windows installer generation.

---

## 🚀 Features

### 🖼️ Image & Photo Tools

Manage and convert image files through a dedicated workspace.

- Image format conversion.
- Image processing and compression.
- Support for multiple image formats.
- Simple file selection and output workflow.

### 🎬 Video Conversion

Convert video files using the FFmpeg processing engine.

- Video format conversion.
- Media stream processing.
- FFprobe-based media inspection.
- Background conversion workflow.

### 🎵 Audio Conversion

Process and convert audio files between supported formats.

- Audio format conversion.
- Audio stream processing.
- FFmpeg-powered encoding.
- Background processing.

### 📦 Media Compression

Reduce media file sizes through dedicated compression tools.

- Image compression.
- Media compression workflow.
- Configurable processing options.

### 🧮 Scientific Calculator

A built-in calculator with standard and scientific operations.

- Basic arithmetic.
- Trigonometric functions.
- Logarithmic functions.
- Exponential and power operations.
- Degree and radian modes.
- Mathematical constants.

### 🎨 Custom Themes

Personalize the application interface with multiple visual themes.

| Theme | Description |
| :--- | :--- |
| Light | Clean light appearance |
| Dark | Dark interface |
| Bochi | Character-inspired theme |
| Frieren | Character-inspired theme |
| Kurumi | Character-inspired theme |
| Mao Mao | Character-inspired theme |
| Ram & Rem | Character-inspired theme |

---

## 🧰 Technology Stack

| Technology | Purpose |
| :--- | :--- |
| C++17 | Core application development |
| Qt 6.12 | Desktop GUI framework |
| Qt Widgets | User interface components |
| CMake | Build system |
| MinGW | C++ compiler |
| FFmpeg 9.0.1 | Audio and video processing |
| FFprobe | Media information and inspection |
| PowerShell | Development automation |
| Inno Setup 7 | Windows installer |

---

## 🏗️ Project Architecture

Marhaban Converter uses a modular project structure to separate user interface components, application services, utilities, and background processing.

```text
marhaban-app/
│
├── .vscode/
│
├── build/
│
├── external/
│   └── ffmpeg/
│
├── installer/
│   ├── payload/
│   ├── output/
│   └── MarhabanConverter.iss
│
├── resources/
│   ├── icons/
│   ├── images/
│   └── styles/
│
├── src/
│   ├── services/
│   ├── ui/
│   ├── utils/
│   ├── workers/
│   │
│   ├── app_context.cpp
│   ├── app_context.h
│   └── main.cpp
│
├── CMakeLists.txt
├── run.ps1
└── README.md
```

### Architecture Overview

- **UI Layer** — Handles application windows, navigation, and user interaction.
- **Services Layer** — Contains media conversion, compression, device, and application services.
- **Workers Layer** — Handles background processing tasks.
- **Utils Layer** — Provides shared utilities and application path management.
- **Resources** — Stores icons, images, and visual assets.

---

## 💻 System Requirements

### Runtime

| Requirement | Specification |
| :--- | :--- |
| Operating System | Windows 10 / Windows 11 |
| Architecture | 64-bit (x64) |
| RAM | 4 GB minimum recommended |
| Storage | Depends on application and media files |
| Dependencies | Bundled Qt, MinGW runtime, FFmpeg |

### Development Environment

- Windows 10 or Windows 11.
- C++17 compatible compiler.
- Qt 6.12.
- CMake.
- MinGW toolchain compatible with Qt.
- FFmpeg and FFprobe.
- Inno Setup 7.

---

## 📥 Installation

### Windows Installer

1. Open the [Releases](https://github.com/kuliahIMC/marhaban-converter/releases) page.
2. Download the latest Marhaban Converter installer.
3. Run the installer.
4. Follow the installation instructions.
5. Launch Marhaban Converter from the Start Menu or Desktop shortcut.

> The Windows installer includes the application runtime dependencies and bundled FFmpeg components.

---

## 🛠️ Build from Source

### 1. Clone the Repository

```bash
git clone https://github.com/kuliahIMC/marhaban-converter.git
cd marhaban-converter
```

### 2. Configure the Project

```powershell
cmake -S . -B build -G "MinGW Makefiles"
```

Make sure CMake can locate your Qt installation and compatible MinGW compiler.

### 3. Build the Application

```powershell
cmake --build build --config Release
```

### 4. Run the Application

```powershell
.\build\MarhabanConverter.exe
```

Alternatively, use the provided PowerShell script:

```powershell
.\run.ps1
```

---

## 📦 Building the Windows Installer

Marhaban Converter uses **Inno Setup 7** to package the application into a Windows installer.

The packaging process includes:

- Application executable.
- Qt runtime libraries.
- MinGW runtime libraries.
- Qt platform and image plugins.
- Application resources.
- FFmpeg and FFprobe binaries.
- Required FFmpeg shared libraries.

Compile the installer using:

```powershell
& "C:\Program Files\Inno Setup 7\ISCC.exe" ".\installer\MarhabanConverter.iss"
```

The generated installer will be available in:

```text
installer/output/
```

---

## ⚖️ License & Third-Party Notices

Marhaban Converter is proprietary software developed by **Marhaban Studio**.

This project uses third-party technologies and libraries, including:

- **Qt** — Qt licensing terms apply to the distributed components.
- **FFmpeg** — The bundled FFmpeg build enables GPL and additional third-party libraries. Distribution must comply with applicable licensing requirements.
- **MinGW** — Runtime libraries are subject to their respective licenses.

Third-party license notices and required source-code availability must be provided in accordance with the applicable licenses.

The Marhaban Converter application itself is not licensed under the same terms as its third-party dependencies.

---

## 🏢 Developer

<p align="center">
  <strong>MARHABAN STUDIO</strong>
</p>

<p align="center">
  Desktop Application Development
</p>

<p align="center">
  Building practical software with modern technologies.
</p>

---

<p align="center">
  <strong>Marhaban Converter</strong><br>
  <sub>Built with C++ · Qt 6 · FFmpeg</sub>
</p>
