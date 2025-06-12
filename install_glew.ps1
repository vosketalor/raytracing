# install_glew.ps1
$ErrorActionPreference = "Stop"

$EXTERNAL_DIR = "external"
$GLEW_VERSION = "2.2.0"
$GLEW_ARCHIVE = "glew-$GLEW_VERSION.tgz"
$GLEW_URL = "https://sourceforge.net/projects/glew/files/glew/$GLEW_VERSION/$GLEW_ARCHIVE/download"

# Créer le dossier external s'il n'existe pas
if (!(Test-Path -Path $EXTERNAL_DIR)) {
    New-Item -ItemType Directory -Path $EXTERNAL_DIR | Out-Null
}
Set-Location $EXTERNAL_DIR

# Télécharger glew si pas déjà présent
if (!(Test-Path -Path $GLEW_ARCHIVE)) {
    Write-Host "Téléchargement de GLEW $GLEW_VERSION..."
    Invoke-WebRequest -Uri $GLEW_URL -OutFile $GLEW_ARCHIVE
} else {
    Write-Host "Archive $GLEW_ARCHIVE déjà présente, pas de téléchargement."
}

# Supprimer l'ancien dossier glew s'il existe
if (Test-Path -Path "glew") {
    Write-Host "Suppression de l'ancien dossier glew..."
    Remove-Item -Recurse -Force "glew"
}

# Extraire l'archive
Write-Host "Extraction de l'archive..."
# Utilise tar inclus avec Git Bash ou Windows 10+
tar -xzf $GLEW_ARCHIVE

# Supprimer l'archive après extraction
Remove-Item $GLEW_ARCHIVE

# Renommer le dossier
Rename-Item -Path "glew-$GLEW_VERSION" -NewName "glew"

# Créer CMakeLists.txt dans glew/
@"
cmake_minimum_required(VERSION 3.16)
project(glew_static)

add_library(glew STATIC
    src/glew.c
)

target_include_directories(glew PUBLIC
    include
)

target_compile_definitions(glew PUBLIC GLEW_STATIC)
"@ | Set-Content -Path "glew/CMakeLists.txt"

Write-Host "Installation de GLEW terminée."
