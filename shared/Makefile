CC=../build/gcc/bin/arm-linux-gnueabihf-g++
CFLAGS=-Wall -Wextra -g -std=c++20 -I ../build/include -I ./include
AR=../build/gcc/arm-linux-gnueabihf/bin/ar
ARFLAGS=rsc

OUT=../build/lib/libshared.a
SUBDIRS=$(shell cd src/ && find * -type d -printf "%p/\n")
MKSUBDIRS=$(addprefix ../build/obj/shared/, $(SUBDIRS_CAR))
SRCS=$(shell cd src/ && find * -type f -name '*.cpp')
OBJS=$(addprefix ../build/obj/shared/, $(SRCS:.cpp=.o))

.PHONY:
all: ../build/obj/shared $(MKSUBDIRS) $(OUT) headers

$(OUT): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

.PHONY: headers
headers:
	mkdir -p ../build/include/shared
	cp -R include/* ../build/include/shared

# Mkdir template
define mk_subdir
../build/obj/shared$(1):
	mkdir -p $$@
endef

# Build template
define compile_subdir
../build/obj/shared/$(1)%.o: src/$(1)%.cpp
	$(CC) $(CFLAGS) -c $$< -o $$@
endef

# Build root
$(eval $(call mk_subdir,))
$(eval $(call compile_subdir,))

# Build subdirectories
$(foreach subdir, $(SUBDIRS), $(eval $(call mk_subdir,$(subdir))))
$(foreach subdir, $(SUBDIRS), $(eval $(call compile_subdir,$(subdir))))

.PHONY: clean
clean:
	rm -rf ../build/obj/shared
	rm ../build/lib/libshared.a
