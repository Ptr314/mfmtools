#!/bin/bash

ARCHITECTURE="x86_64"
PLATFORM="linux"
QT_PATH="~/Qt/6.8.1-static"
BUILD_DIR="./build/${PLATFORM}_${ARCHITECTURE}"
RELEASE_DIR="./release/linux"

VERSION=`cat ../src/globals.h | grep 'PROJECT_VERSION' | awk '{printf $3}' | tr -d '"'`

cmake -DCMAKE_PREFIX_PATH="$QT_PATH" -S ../src -B "$BUILD_DIR" -G Ninja

cwd=$(pwd)
cd $BUILD_DIR
ninja
cd $cwd

mkdir $RELEASE_DIR
#cp "${BUILD_DIR}/MFMTools.dmg" "${RELEASE_DIR}/mfmtools_${VERSION}_${PLATFORM}_${ARCHITECTURE}.dmg"