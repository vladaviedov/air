GPIOD_LIB=build/lib/libgpiodcxx.so

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

.PHONY: driver
driver: $(GPIOD_LIB)
	$(MAKE) -C driver

.PHONY: car
car: driver
	$(MAKE) -C car

.PHONY: control
control: driver
	$(MAKE) -C control

.PHONY: clean
clean:
	$(MAKE) -C driver clean
	$(MAKE) -C car clean

.PHONY: libclean
libclean:
	$(MAKE) -C lib/libgpiod clean

.PHONY: fullclean
fullclean:
	rm -rf build
