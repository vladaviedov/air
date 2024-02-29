GPIOD_LIB=build/lib/libgpiodcxx.so

.PHONY: all
all: builddirs car

.PHONY: builddirs
builddirs:
	mkdir -p build/
	mkdir -p build/lib
	mkdir -p build/include
	mkdir -p build/obj
	mkdir -p build/bin

$(GPIOD_LIB): lib/libgpiod
	cd $^; \
	./autogen.sh \
		--prefix=${CURDIR}/build \
		--enable-bindings-cxx; \
	make; \
	make install

.PHONY: driver
driver: $(GPIOD_LIB)
	$(MAKE) -C driver

.PHONY: car
car: driver
	$(MAKE) -C car

.PHONY: clean
clean:
	rm -rf build

.PHONY: libclean
libclean:
	$(MAKE) -C lib/libgpiod clean
