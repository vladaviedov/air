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
cd compiler/binutils-gdb
mkdir build
cd build
../configure \
	--prefix=$PREFIX \
	--target=arm-linux-gnueabihf \
	--with-arch=armv6 \
	--with-fpu=vpf \
	--with-float=hard \
	--disable-gdb \
	--disable-werror
make $THREAD_OPT
make install
cd $BASE

# Step 3: Begin compiling gcc
cd compiler/gcc
contrib/download_prerequisites
./configure \
	--prefix=$PREFIX \
	--target=arm-linux-gnueabihf \
	--enable-languages=c,c++ \
	--with-arch=armv6 \
	--with-fpu=vfp \
	--with-float=hard \
	--disable-gcov \
	--enable-threads \
	--with-glibc-version=2.36
make $THREAD_OPT all-gcc
make install-gcc
cd $BASE

# Step 4: Begin compiling glibc
cd compiler/glibc
mkdir build
cd build
../configure \
	--prefix=$PREFIX \
	--build=$MACHTYPE \
	--host=arm-linux-gnueabihf \
	--target=arm-linux-gnueabihf \
	--with-arch=armv6 \
	--with-fpu=vfp \
	--with-float=hard \
	--disable-werror \
	libc_cv_forced_unwind=yes
