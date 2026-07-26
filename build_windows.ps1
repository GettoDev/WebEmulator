# Build script for Vircon32 Web Emulator on Windows
# Requires Emscripten SDK to be installed and activated

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Vircon32 Web Emulator Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Intenta cargar Emscripten automáticamente si no está en el PATH de la sesión
if (-not (Get-Command "emcmake" -ErrorAction SilentlyContinue)) {
    if (Test-Path "C:\emsdk\emsdk_env.ps1") {
        Write-Host "Cargando entorno Emscripten..." -ForegroundColor Yellow
        & "C:\emsdk\emsdk_env.ps1"
    }
}

# Check if emcmake is available
if (Get-Command "emcmake" -ErrorAction SilentlyContinue) {
    Write-Host "Emscripten found successfully!" -ForegroundColor Green
} else {
    Write-Host "ERROR: Emscripten not found!" -ForegroundColor Red
    Write-Host "Please install Emscripten SDK and activate it using emsdk_env.bat" -ForegroundColor Yellow
    Write-Host "Download from: https://emscripten.org/docs/getting_started/downloads.html" -ForegroundColor Yellow
    exit 1
}

# Set paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ScriptDir "build"
$InstallDir = Join-Path $ScriptDir "output"

Write-Host "Source directory: $ScriptDir" -ForegroundColor Gray
Write-Host "Build directory: $BuildDir" -ForegroundColor Gray
Write-Host "Output directory: $InstallDir" -ForegroundColor Gray
Write-Host ""

# Clean previous build
if (Test-Path $BuildDir) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

if (Test-Path $InstallDir) {
    Write-Host "Cleaning previous output..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $InstallDir
}

# Create build directory
Write-Host "Creating build directory..." -ForegroundColor Gray
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

# Configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor Gray
Push-Location $BuildDir
try {
    emcmake cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release $ScriptDir
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
} finally {
    Pop-Location
}

# Build
Write-Host "Building..." -ForegroundColor Gray
Push-Location $BuildDir
try {
    emmake cmake --build . --config Release
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
} finally {
    Pop-Location
}

# Create output directory
Write-Host "Creating output directory..." -ForegroundColor Gray
New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null

# Copy files to output
Write-Host "Copying files to output..." -ForegroundColor Gray
Copy-Item -Path "$BuildDir/Vircon32Web.html" -Destination $InstallDir -Force
Copy-Item -Path "$BuildDir/Vircon32Web.js" -Destination $InstallDir -Force
Copy-Item -Path "$BuildDir/Vircon32Web.wasm" -Destination $InstallDir -Force
if (Test-Path "$BuildDir/Vircon32Web.data") {
    Copy-Item -Path "$BuildDir/Vircon32Web.data" -Destination $InstallDir -Force
}
Copy-Item -Path "$BuildDir/web_interface.js" -Destination $InstallDir -Force
if (Test-Path "$ScriptDir/logo.png") {
    Copy-Item -Path "$ScriptDir/logo.png" -Destination $InstallDir -Force
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Output files are in: $InstallDir" -ForegroundColor Cyan
Write-Host ""
Write-Host "To test the emulator:" -ForegroundColor Yellow
Write-Host "1. Start a local web server in the output directory" -ForegroundColor Gray
Write-Host "   Example: python -m http.server 8000" -ForegroundColor Gray
Write-Host "2. Open http://localhost:8000 in your browser" -ForegroundColor Gray
Write-Host ""
