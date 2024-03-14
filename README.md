# air

## Target Info
Raspberry Pi Zero W 1.1

- OS: Raspbian 12
- Kernel: Linux 6.1.0-rpi7-rpi-v6
- GCC: 12.2
- GLIBC: 2.36
- Binutils: 2.40

## Requirements
Archlinux Packages:
```
base-devel
autoconf-archive
python
rsync
```

## Setup
Get submodules if you have not done so
```
git submodule init
git submodule update
```

You need a cross-compiler for ARMv6 with threading and C++ support.
Run `compiler.sh` to build a properly configured cross-compiler (takes a while).

Build tested with:
- gcc 13, x86_64, archlinux

## Compile
Compile with make
```
make
```

Clean commands:
- `make clean` - clean `air` code
- `make libclean` - clean libraries
- `make fullclean` - clean everything including compiler

## Binaries
Output files:
```
build/bin/car
build/bin/control
```

## Runtime Dependencies
Raspbian Packages:
```
libgpiod2
```
