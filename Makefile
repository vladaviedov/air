GPIOD_LIB=build/lib/libgpiodcxx.so
I2C_LIB=build/lib/libi2c.so
AIR_DIR=$(shell pwd)

.PHONY: all
all: builddirs car control

.PHONY: builddirs
builddirs:
	mkdir -p build/
	mkdir -p build/lib
	mkdir -p build/include
	mkdir -p build/obj
	mkdir -p build/bin

$(GPIOD_LIB): lib/libgpiod
	./libgpiod.sh -j$$(nproc)

$(I2C_LIB): lib/i2c-tools
	$(MAKE) -C lib/i2c-tools \
		BUILD_STATIC_LIB=0 \
		CC=$(AIR_DIR)/build/gcc/bin/arm-linux-gnueabihf-gcc \
		AR=$(AIR_DIR)/build/gcc/bin/arm-linux-gnueabihf-ar  \
		STRIP=$(AIR_DIR)/build/gcc/bin/arm-linux-gnueabihf-strip 
	$(MAKE) -C lib/i2c-tools \
		install \
		PREFIX=$(AIR_DIR)/build/

.PHONY: driver
driver: $(GPIOD_LIB) $(I2C_LIB)
	$(MAKE) -C driver

.PHONY: shared
shared: driver
	$(MAKE) -C shared

.PHONY: car
car: driver shared
	$(MAKE) -C car

.PHONY: control
control: driver shared
	$(MAKE) -C control

.PHONY: clean
clean:
	$(MAKE) -C driver clean
	$(MAKE) -C car clean

.PHONY: libclean
libclean:
	rm -rf lib/libgpiod/build
	rm $(GPIOD_LIB)
	$(MAKE) -C lib/i2c-tools clean
	rm $(I2C_LIB)

.PHONY: fullclean
fullclean: libclean
	rm -rf build
	rm -rf compiler/binutils-gdb/build
	rm -rf compiler/gcc/build
	rm -rf compiler/glibc/build

.PHONY: format
format:
	$(MAKE) -C driver format
	$(MAKE) -C shared format
	$(MAKE) -C car format
	$(MAKE) -C control format

# Quality checks
.PHONY: runlint
runlint:
	$(MAKE) -C driver headers
	$(MAKE) -C shared headers
	$(MAKE) -C driver runlint
	$(MAKE) -C shared runlint
	$(MAKE) -C car runlint
	$(MAKE) -C control runlint

.PHONY: checkformat
checkformat:
	$(MAKE) -C driver checkformat
	$(MAKE) -C shared checkformat
	$(MAKE) -C car checkformat
	$(MAKE) -C control checkformat
