#!/bin/sh
set -e

# Directories
BASE=$(pwd)
BUILD=$BASE/build
PREFIX=$BUILD/gcc

# Variables
JOPT=$1
if [ "$JOPT" = "" ]; then
	read -p "-j flag is not set. Are you sure? [y/N]: " ans
	[ "$ans" = y ] || [ "$ans" = Y ] || exit 1
fi

# Create dirs
mkdir -p $BUILD
mkdir -p $PREFIX

# args:
# 	check file
#	name of step
#	build function
cond_build() {
	if [ ! -f $1 ]; then
		echo Running: $2
		$3 $1
	else
		echo Skipping: $2
	fi
}

# Step 1: Get kernel headers
headers() {
	cd compiler/linux
	export KERNEL=kernel
	make ARCH=arm INSTALL_HDR_PATH=$PREFIX/arm-linux-gnueabihf headers_install
	cd $BASE
	touch $1
}


# Step 2: Compile binutils
binutils() {
	cd compiler/binutils-gdb
	mkdir -p build
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
	touch $1
}

# Step 3: Begin compiling gcc
gcc_stage1() {
	cd compiler/gcc
	contrib/download_prerequisites
	mkdir -p build
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
	touch $1
}

# Step 4: Begin compiling glibc
glibc_stage1() {
	cd compiler/glibc
	mkdir -p build
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
	mkdir -p $PREFIX/arm-linux-gnueabihf/include/gnu
	touch $PREFIX/arm-linux-gnueabihf/include/gnu/stubs.h
	cd $BASE
	touch $1
}

# Step 5: Compile libgcc
gcc_stage2() {
	cd compiler/gcc/build
	make $JOPT all-target-libgcc
	make install-target-libgcc
	cd $BASE
	touch $1
}

# Step 6: Finish compiling glibc
glibc_stage2() {
	cd compiler/glibc/build
	make $JOPT
	make install
	cd $BASE
	touch $1
}

# Step 7: Finish building gcc
gcc_final() {
	cd compiler/gcc/build
	make $JOPT
	make install
	cd $BASE
	touch $1
}

# Build
cond_build $BUILD/.compiler_headers "copy kernel headers" headers
cond_build $BUILD/.compiler_binutils "binutils" binutils
cond_build $BUILD/.compiler_gcc_stage1 "gcc stage 1" gcc_stage1
cond_build $BUILD/.compiler_glibc_stage1 "glibc stage 1" glibc_stage1
cond_build $BUILD/.compiler_gcc_stage2 "gcc stage 2" gcc_stage2
cond_build $BUILD/.compiler_glibc_stage2 "glibc stage 2" glibc_stage2
cond_build $BUILD/.compiler_gcc_final "gcc final stage" gcc_final
