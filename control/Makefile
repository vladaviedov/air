CC=../build/gcc/bin/arm-linux-gnueabihf-g++
CFLAGS=-Wall -Wextra -g -std=c++20 -I ../build/include
LDFLAGS=-L ../build/lib/ -ldriver -lshared -lgpiod -lgpiodcxx -li2c

OUT=../build/bin/control
SUBDIRS=$(shell cd src/ && find * -type d -printf "%p/\n")
MKSUBDIRS=$(addprefix ../build/obj/control/, $(SUBDIRS))
SRCS=$(shell cd src/ && find * -type f -name '*.cpp')
OBJS=$(addprefix ../build/obj/control/, $(SRCS:.cpp=.o))

FORMAT=clang-format
FORMAT_FIX_FLAGS=-i
FORMAT_CHECK_FLAGS=--dry-run --Werror
LINT=clang-tidy
LINT_FLAGS=--quiet

.PHONY:
all: ../build/obj/control $(MKSUBDIRS) $(OUT)

.PHONY: $(OUT)
$(OUT): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Mkdir template
define mk_subdir
../build/obj/control$(1):
	mkdir -p $$@
endef

# Build template
define compile_subdir
../build/obj/control/$(1)%.o: src/$(1)%.cpp
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
	rm -rf ../build/obj/control
	rm ../build/bin/control

SRC_DIR_FILES=$(shell find src -type f)

.PHONY: format
format:
	$(FORMAT) $(FORMAT_FIX_FLAGS) $(SRC_DIR_FILES)

# Quality checks
.PHONY: checklint
runlint:
	$(LINT) $(LINT_FLAGS) $(SRC_DIR_FILES)

.PHONY: checkformat
checkformat:
	$(FORMAT) $(FORMAT_CHECK_FLAGS) $(SRC_DIR_FILES)
