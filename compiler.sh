set -e

# Directories
BASE=$(pwd)
BUILD=$BASE/build
PREFIX=$BUILD/gcc

# Variables
JOPT=$1
if [[ $JOPT == "" ]]; then
	read -p "-j flag is not set. Are you sure? [y/N]: " ans
	[[ $ans == [yY] ]] || exit 1
fi

# Create dirs
mkdir -p $BUILD
mkdir -p $PREFIX

# Step 1: Get kernel headers
cd compiler/linux
export KERNEL=kernel
make ARCH=arm INSTALL_HDR_PATH=$PREFIX/arm-linux-gnueabihf headers_install
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
	--disable-werror \
	--disable-multilib
make $JOPT
make install
cd $BASE

# Step 3: Begin compiling gcc
cd compiler/gcc
contrib/download_prerequisites
mkdir build
cd build
../configure \
	--prefix=$PREFIX \
	--target=arm-linux-gnueabihf \
	--enable-languages=c,c++ \
	--with-arch=armv6 \
	--with-fpu=vfp \
	--with-float=hard \
	--disable-gcov \
	--enable-threads \
	--with-glibc-version=2.36 \
	--disable-multilib \
	--disable-libsanitizer
make $JOPT all-gcc
make install-gcc
cd $BASE

# Step 4: Begin compiling glibc
cd compiler/glibc
mkdir build
cd build
CC=$PREFIX/bin/arm-linux-gnueabihf-gcc \
	LD=$PREFIX/bin/arm-linux-gnueabihf-ld \
	AR=$PREFIX/bin/arm-linux-gnueabihf-ar \
	RANLIB=$PREFIX/bin/arm-linux-gnueabihf-ranlib \
	../configure \
	--prefix=$PREFIX/arm-linux-gnueabihf \
	--build=$MACHTYPE \
	--host=arm-linux-gnueabihf \
	--target=arm-linux-gnueabihf \
	--with-arch=armv6 \
	--with-fpu=vfp \
	--with-float=hard \
	--disable-profile \
	--disable-werror \
	--disable-multilib \
	libc_cv_forced_unwind=yes \
	libc_cv_cxx_link_ok=no
make install-bootstrap-headers=yes install-headers
make $JOPT csu/subdir_lib
install csu/crt1.o csu/crti.o csu/crtn.o $PREFIX/arm-linux-gnueabihf/lib
$PREFIX/bin/arm-linux-gnueabihf-gcc \
	-nostdlib \
	-nostartfiles \
	-shared \
	-x c /dev/null \
	-o $PREFIX/arm-linux-gnueabihf/lib/libc.so
mkdir $PREFIX/arm-linux-gnueabihf/include/gnu
touch $PREFIX/arm-linux-gnueabihf/include/gnu/stubs.h
cd $BASE

# Step 5: Compile libgcc
cd compiler/gcc/build
make $JOPT all-target-libgcc
make install-target-libgcc
cd $BASE

# Step 6: Finish compiling glibc
cd compiler/glibc/build
make $JOPT
make install
cd $BASE

# Step 7: Finish building gcc
cd compiler/gcc/build
make $JOPT
make install
