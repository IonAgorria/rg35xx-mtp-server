#!/usr/bin/bash

set -e

if [ ! -f "CMakeLists.txt" ]; then
  echo "Wrong dir no cmake"
  exit 1
fi

BUILD_DIR=./build
mkdir -p $BUILD_DIR

cmake \
-B"$BUILD_DIR" \
-H"." \
-G"Unix Makefiles" \
-DBOOST_ROOT="/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/" \
-DCMAKE_INSTALL_PREFIX="./$BUILD_DIR/out" \
-DCMAKE_BUILD_TYPE=Release \
"$@"


cd $BUILD_DIR
make -j4
make install
