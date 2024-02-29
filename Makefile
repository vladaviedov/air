GPIOD_LIB=build/lib/lib/libgpiodcxx.so

.PHONY: all
all: builddirs car

.PHONY: builddirs
builddirs:
	mkdir -p build/
	mkdir -p build/lib
	mkdir -p build/obj
	mkdir -p build/bin

$(GPIOD_LIB): lib/libgpiod
	cd $^; \
	./autogen.sh \
		--prefix=${CURDIR}/build/lib/ \
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
