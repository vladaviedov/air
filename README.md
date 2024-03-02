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
```

## Setup
Get submodules if you have not done so
```
git submodule init
git submodule update
```

You need a cross-compiler for ARMv6 with threading and C++ support.
Run `compiler.sh` to build a properly configured cross-compiler (takes a while).

## Compile
Compile with make
```
make
```

Output files:
```
build/bin/car
```

Copy over to the pi
