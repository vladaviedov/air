BASE=$(pwd)
THREAD_OPT=$1

cd lib/libgpiod
mkdir build
cd build
export PATH=$PATH:$BASE/build/gcc/bin:$BASE/build/gcc/arm-linux-gnueabihf/bin
../autogen.sh \
	--prefix=$BASE/build \
	--host=arm-linux-gnueabihf \
	--target=arm-linux-gnueabihf \
	--enable-bindings-cxx
make $THREAD_OPT
make install
