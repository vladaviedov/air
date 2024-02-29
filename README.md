# air

## Microcontroller Info
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

## Compile
Get submodule if you have not done so
```
git submodule init
git submodule update
```

Compile with make
```
make
```

Output files:
```
build/bin/car
```
