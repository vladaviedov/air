# air
Proof of concept for Autonomous Intersection Regulation. There is one control
device and multiple car devices. Communication is done via a RF module over
70 cm using GFSK with time slots.

`shared` and `driver` have stable code, but `control` and `car` have unfinished
components, due to the deadline.

## Target Info
Raspberry Pi Zero W 1.1

- OS: Raspbian 12
- Kernel: Linux 6.1.0-rpi7-rpi-v6
- GCC: 12.2
- GLIBC: 2.36
- Binutils: 2.40

## Development

### Requirements
Archlinux Packages:
```
autoconf-archive
base-devel
python
rsync
```

Debian Packages:
```
autoconf-archive
automake
bison
build-essential
curl
flex
gawk
libtool
pkg-config
python3
rsync
texinfo
```

### Setup
Get submodules if you have not done so
```
git submodule init
git submodule update
```

You need a cross-compiler for ARMv6 with threading and C++ support.
Run `compiler.sh` to build a properly configured cross-compiler (takes a while).

Build tested with:
- gcc 13, x86_64, archlinux
- gcc 12, x86_64, debian 12

### Compile
Compile with make
```
make
```

Clean commands:
- `make clean` - clean `air` code
- `make libclean` - clean libraries
- `make fullclean` - clean everything including compiler

### Tools
Formatter:
- `clang-format` - version 17
- `make checkformat` - check formatting
- `make format` - run formatter task

Static analyzer:
- `clang-tidy` - version 17
- `make runlint` - run static analyzer

On debian/ubuntu, these are not available in repo currently, so get it using
the LLVM script.

### Binaries
Output files:
```
build/bin/car
build/bin/control
```

## Usage

### Runtime Dependencies
Raspbian Packages:
```
libgpiod2
i2c-tools
```

### Setup
- `/etc/airid`

This file should be readable and contain the ID of the car. This should be a
maximum of 15 bytes and used for identification over radio. For example, this
might contain your FCC callsign if operating in the amateur bands.

- `/etc/air/profile`

This file should be created before execution. It should be writable by the
user. Calibration data is stored and read from this file by default.

## About Notice
This notice is included in the built binaries.
```
AIR Car & Control Software
Copyright (C) 2024 Vladyslav Aviedov, Caio DaSilva, Scott Abramson

This program is part of a project for GE1502 Cornerstone of Engineering 2 at
Northeastern University (Spring 2024). Project group members:
- Aidan Hanson
- Caio DaSilva
- Scott Abramson
- Vladyslav Aviedov

Source code: https://github.com/vladaviedov/air
License: GNU General Public License v2.0

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by the Free
Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
```
