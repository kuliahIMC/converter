$ErrorActionPreference = "Stop"

$env:PATH = "C:\Qt\6.12.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:PATH"

cmake --build build

if ($LASTEXITCODE -ne 0) {
    throw "Build gagal."
}

& ".\build\MarhabanConverter.exe"