#!/bin/bash
set -e

EXTERNAL_DIR="external"
GLEW_VERSION="2.2.0"
GLEW_ARCHIVE="glew-${GLEW_VERSION}.tgz"
GLEW_URL="https://sourceforge.net/projects/glew/files/glew/${GLEW_VERSION}/${GLEW_ARCHIVE}/download"

mkdir -p "$EXTERNAL_DIR"
cd "$EXTERNAL_DIR"

# Télécharger glew si pas déjà présent
if [ ! -f "$GLEW_ARCHIVE" ]; then
    echo "Téléchargement de GLEW $GLEW_VERSION..."
    wget -O "$GLEW_ARCHIVE" "$GLEW_URL"
else
    echo "Archive $GLEW_ARCHIVE déjà présente, pas de téléchargement."
fi

# Supprimer l'ancien dossier si existant
if [ -d "glew" ]; then
    echo "Suppression de l'ancien dossier glew..."
    rm -rf glew
fi

# Extraire l'archive
echo "Extraction de l'archive..."
tar -xzf "$GLEW_ARCHIVE"

# Supprimer l'archive .tgz après extraction
rm "$GLEW_ARCHIVE"

# Renommer le dossier extrait
mv "glew-${GLEW_VERSION}" glew

# Créer CMakeLists.txt dans glew/
cat > glew/CMakeLists.txt << EOF
cmake_minimum_required(VERSION 3.16)
project(glew_static)

add_library(glew STATIC
    src/glew.c
)

target_include_directories(glew PUBLIC
    include
)

target_compile_definitions(glew PUBLIC GLEW_STATIC)
EOF

echo "Installation de GLEW terminée."
