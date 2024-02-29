CC=g++
CFLAGS=-Wall -Wextra -g -std=c++20 -I ../build/include
LDFLAGS=-L../build/lib/ -ldriver -lgpiodcxx

OUT=../build/bin/car
SUBDIRS=$(shell cd src/ && find * -type d -printf "%p/\n")
MKSUBDIRS=$(addprefix ../build/obj/car/, $(SUBDIRS_CAR))
SRCS=$(shell cd src/ && find * -type f -name '*.cpp')
OBJS=$(addprefix ../build/obj/car/, $(SRCS:.cpp=.o))

.PHONY:
all: ../build/obj/car $(MKSUBDIRS) $(OUT)

$(OUT): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Mkdir template
define mk_subdir
../build/obj/car$(1):
	mkdir -p $$@
endef

# Build template
define compile_subdir
../build/obj/car/$(1)%.o: src/$(1)%.cpp
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
	rm -rf ../build/obj/car
	rm ../build/bin/car
