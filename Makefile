CC=g++ -std=c++20
CFLAGS=-Wall -Wextra -g
LDFLAGS=-lgpiodcxx

GPIOD_LIB=libgpiodcxx.so

OUT_DRIVER=build/bin/driver.so
SUBDIRS_DRIVER=$(shell cd src/driver && find * -type d -printf "%p/\n")
MKSUBDIRS_DRIVER=$(addprefix build/obj/driver/, $(SUBDIRS_CAR))
SRCS_DRIVER=$(shell cd src/driver && find * -type f -name '*.cpp')
OBJS_DRIVER=$(addprefix build/obj/driver/, $(SRCS_DRIVER:.cpp=.o))

OUT_CAR=build/bin/car
SUBDIRS_CAR=$(shell cd src/car && find * -type d -printf "%p/\n")
MKSUBDIRS_CAR=$(addprefix build/obj/car/, $(SUBDIRS_CAR))
SRCS_CAR=$(shell cd src/car && find * -type f -name '*.cpp')
OBJS_CAR=$(addprefix build/obj/car/, $(SRCS_CAR:.cpp=.o))

.PHONY:
all: rootdirs $(GPIOD_LIB) $(MKSUBDIRS) $(OUT_DRIVER) $(OUT_CAR)

$(GPIOD_LIB): lib/libgpiod
	cd $^; \
	./autogen.sh \
		--prefix=${CURDIR}/build/lib/ \
		--enable-bindings-cxx; \
	make; \
	make install

$(OUT_DRIVER): $(OBJS_DRIVER)
	$(CC) -shared $^ -o $@ $(LDFLAGS)

$(OUT_CAR): $(OBJS_CAR)
	$(CC) $^ -o $@ $(LDFLAGS)

.PHONY: rootdirs
rootdirs: build/bin build/lib build/obj/car build/obj/driver

build/bin:
	mkdir -p $@

build/lib:
	mkdir -p $@

# Mkdir template
define mk_subdir
build/obj/$(1):
	mkdir -p $$@
endef

# Build template
define compile_subdir
build/obj/$(1)%.o: src/$(1)%.cpp
	$(CC) $(CFLAGS) -c $$< -o $$@
endef

## Driver section

# Build driver root
$(eval $(call mk_subdir,driver))
$(eval $(call compile_subdir,driver))

# Build driver subdirectories
$(foreach subdir, $(SUBDIRS_DRIVER), $(eval $(call mk_subdir,$(subdir))))
$(foreach subdir, $(SUBDIRS_DRIVER), $(eval $(call compile_subdir,$(subdir))))

## Car section

# Build car root
$(eval $(call mk_subdir,car))
$(eval $(call compile_subdir,car))

# Build car subdirectories
$(foreach subdir, $(SUBDIRS_CAR), $(eval $(call mk_subdir,$(subdir))))
$(foreach subdir, $(SUBDIRS_CAR), $(eval $(call compile_subdir,$(subdir))))

.PHONY: clean
clean:
	rm -rf build
