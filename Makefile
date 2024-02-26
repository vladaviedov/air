CC=g++
CFLAGS=-Wall -Wextra -g
LDFLAGS=-lgpiodcxx

OUT_CAR=build/bin/car
SUBDIRS_CAR=$(shell cd src/car && find * -type d -printf "%p/\n")
MKSUBDIRS_CAR=$(addprefix build/obj/car/, $(SUBDIRS_CAR))
SRCS_CAR=$(shell cd src/car && find * -type f -name '*.cpp')
OBJS_CAR=$(addprefix build/obj/car/, $(SRCS_CAR:.cpp=.o))

.PHONY:
all: rootdirs $(MKSUBDIRS) $(OUT_CAR)

$(OUT_CAR): $(OBJS_CAR)
	$(CC) $^ -o $@ $(LDFLAGS)

.PHONY: rootdirs
rootdirs: build/bin build/obj/car

build/bin:
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

# Build car root
$(eval $(call mk_subdir,car))
$(eval $(call compile_subdir,car))

# Build car subdirectories
$(foreach subdir, $(SUBDIRS_CAR), $(eval $(call mk_subdir,$(subdir))))
$(foreach subdir, $(SUBDIRS_CAR), $(eval $(call compile_subdir,$(subdir))))

.PHONY: clean
clean:
	rm -rf build
