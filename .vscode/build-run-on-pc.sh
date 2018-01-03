#!/bin/bash
set -e

ROOT=$(dirname $(dirname $(readlink -f $0)))
cd $ROOT

echo $ROOT

echo "Clean and create build directory"
rm -rf build-pc
mkdir build-pc

cd build-pc

echo "Running qmake..."
qmake ..

echo "Building..."
make

echo "Launching OpenStore..."
$ROOT/build-pc/openstore/openstore
