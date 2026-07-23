# ThaiOS Docker Build Script (PowerShell)
# Builds the ThaiOS ISO using Docker Desktop

$ErrorActionPreference = "Stop"
$ThaiOSRoot = Split-Path -Parent (Split-Path -Parent $PSCommandPath)
$OutputDir = "$env:USERPROFILE\Desktop\ThaiOS-ISO"

Write-Host "=======================================" -ForegroundColor Cyan
Write-Host "  ThaiOS ISO Builder (Docker)" -ForegroundColor Cyan
Write-Host "=======================================" -ForegroundColor Cyan
Write-Host ""

# Check Docker
try {
    $dockerVer = docker --version
    Write-Host "Docker trovato: $dockerVer" -ForegroundColor Green
} catch {
    Write-Host "ERRORE: Docker non trovato." -ForegroundColor Red
    Write-Host "Scarica Docker Desktop da: https://www.docker.com/products/docker-desktop/" -ForegroundColor Yellow
    exit 1
}

# Check that Docker is running
try {
    $dockerInfo = docker info 2>$null
    if (-not $?) {
        Write-Host "ERRORE: Docker Desktop non e' in esecuzione." -ForegroundColor Red
        Write-Host "Avvia Docker Desktop dalla barra delle applicazioni e riprova." -ForegroundColor Yellow
        exit 1
    }
} catch {
    Write-Host "ERRORE: Docker Desktop non e' in esecuzione." -ForegroundColor Red
    exit 1
}

Write-Host "Build ISO ThaiOS in corso..." -ForegroundColor Yellow
Write-Host "Attenzione: ci vorranno 15-30 minuti." -ForegroundColor Yellow
Write-Host ""

# Build the ISO using Docker
docker build -t thaios-builder -f "$ThaiOSRoot\Dockerfile" "$ThaiOSRoot"

if (-not $?) {
    Write-Host "ERRORE: Build fallita." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Estrazione ISO da Docker..." -ForegroundColor Yellow

# Create output directory
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

# Extract the ISO from the image
$isoPath = "$OutputDir\ThaiOS-1.0.iso"
docker create --name thaios-tmp thaios-builder
docker cp thaios-tmp:/ThaiOS-1.0.iso "$isoPath"
docker rm thaios-tmp > $null

if (Test-Path $isoPath) {
    $size = (Get-Item $isoPath).Length / 1MB
    Write-Host ""
    Write-Host "=======================================" -ForegroundColor Green
    Write-Host "  BUILD COMPLETATO CON SUCCESSO!" -ForegroundColor Green
    Write-Host "=======================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "ISO: $isoPath" -ForegroundColor White
    Write-Host "Dimensione: $([math]::Round($size, 1)) MB" -ForegroundColor White
    Write-Host ""
    Write-Host "Per creare una chiavetta USB avviabile:" -ForegroundColor Cyan
    Write-Host "  1. Scarica Rufus: https://rufus.ie" -ForegroundColor Gray
    Write-Host "  2. Apri Rufus, seleziona l'ISO e la chiavetta" -ForegroundColor Gray
    Write-Host "  3. Clicca AVVIA" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Oppure monta l'ISO direttamente in VirtualBox." -ForegroundColor Cyan
} else {
    Write-Host "ERRORE: ISO non trovata." -ForegroundColor Red
    exit 1
}
