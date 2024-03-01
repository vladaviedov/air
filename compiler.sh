# Directories
BASE=$(pwd)
BUILD=$BASE/build
PREFIX=$BUILD/gcc

# Variables
THREAD_OPT=$1

# Create dirs
mkdir -p $BUILD
mkdir -p $PREFIX

# Step 1: Get kernel headers
cd compiler/linux
export KERNEL=kernel
make \ 
	ARCH=arm \
	INSTALL_HDR_PATH=$PREFIX/arm-linux-gnueabihf headers_install
cd $BASE

# Step 2: Compile binutils
cd compiler/bin
mkdir build
cd build
../configure \
	--prefix=$PREFIX \
	--target=arm-linux-gnueabihf \
	--with-arch=armv6 \
	--with-fpu=vpf \
	--with-float=hard \
	--disable-gdb
make $THREAD_OPT
make install
cd $BASE
