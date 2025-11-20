#!/bin/bash

# Define variables
SRC_DIR="/home/dart/aep/sdk-4.18/examples/demoapp/demoapp"
BUILD_DIR="$SRC_DIR/build"
TARGET="demoapp"
REMOTE_USER="root"
REMOTE_HOST="172.16.36.183"
REMOTE_PATH="/home/dart/"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

# Clean previous build
make clean

# Run qmake
/opt/aep-cdb4v2/sysroots/i686-pokysdk-linux/usr/bin/qt5/qmake "$SRC_DIR/$TARGET.pro" -spec cdb4v2-aep-cortexa9-g++ CONFIG+=debug CONFIG+=qml_debug CONFIG+=AEP-CDB4V2

# Build with make
make 

# Copy to remote host
scp "$TARGET" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_PATH"
