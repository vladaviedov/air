CC=g++
CFLAGS=-Wall -Wextra -g -std=c++20 -I ../build/include -fPIC
AR=ar
ARFLAGS=rusc

OUT=../build/lib/libdriver.a
SUBDIRS=$(shell cd src/ && find * -type d -printf "%p/\n")
MKSUBDIRS=$(addprefix ../build/obj/driver/, $(SUBDIRS_CAR))
SRCS=$(shell cd src/ && find * -type f -name '*.cpp')
OBJS=$(addprefix ../build/obj/driver/, $(SRCS:.cpp=.o))

$(info [${CURDIR}])

.PHONY:
all: ../build/obj/driver $(MKSUBDIRS) $(OUT)

$(OUT): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# Mkdir template
define mk_subdir
../build/obj/driver$(1):
	mkdir -p $$@
endef

# Build template
define compile_subdir
../build/obj/driver/$(1)%.o: src/$(1)%.cpp
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
	rm -rf ../build/obj/driver
	rm ../build/lib/libdriver.so
