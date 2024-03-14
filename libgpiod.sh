#!/bin/sh
set -e

BASE=$(pwd)
JOPT=$1

cd lib/libgpiod
mkdir -p build
cd build
export PATH=$PATH:$BASE/build/gcc/bin:$BASE/build/gcc/arm-linux-gnueabihf/bin
../autogen.sh \
	--prefix=$BASE/build \
	--host=arm-linux-gnueabihf \
	--target=arm-linux-gnueabihf \
	--enable-bindings-cxx
make $JOPT
make install
