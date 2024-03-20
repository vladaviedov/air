CC=../build/gcc/bin/arm-linux-gnueabihf-g++
CFLAGS=-Wall -Wextra -g -std=c++20 -I ../build/include -I ./include
AR=../build/gcc/arm-linux-gnueabihf/bin/ar
ARFLAGS=rsc

OUT=../build/lib/libdriver.a
SUBDIRS=$(shell cd src/ && find * -type d -printf "%p/\n")
MKSUBDIRS=$(addprefix ../build/obj/driver/, $(SUBDIRS_CAR))
SRCS=$(shell cd src/ && find * -type f -name '*.cpp')
OBJS=$(addprefix ../build/obj/driver/, $(SRCS:.cpp=.o))

FORMAT=clang-format
FORMAT_FIX_FLAGS=-i
FORMAT_CHECK_FLAGS=--dry-run --Werror
LINT=clang-tidy
LINT_FLAGS=--quiet

.PHONY:
all: ../build/obj/driver $(MKSUBDIRS) $(OUT) headers

$(OUT): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

.PHONY: headers
headers:
	mkdir -p ../build/include/driver
	cp -R include/* ../build/include/driver

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
	rm ../build/lib/libdriver.a

SRC_DIR_FILES=$(shell find src -type f)
INC_DIR_FILES=$(shell find include -type f)

.PHONY: format
format:
	$(FORMAT) $(FORMAT_FIX_FLAGS) $(INC_DIR_FILES)
	$(FORMAT) $(FORMAT_FIX_FLAGS) $(SRC_DIR_FILES)

# Quality checks
.PHONY: checklint
runlint:
	$(LINT) $(LINT_FLAGS) $(INC_DIR_FILES)
	$(LINT) $(LINT_FLAGS) $(SRC_DIR_FILES)

.PHONY: checkformat
checkformat:
	$(FORMAT) $(FORMAT_CHECK_FLAGS) $(INC_DIR_FILES)
	$(FORMAT) $(FORMAT_CHECK_FLAGS) $(SRC_DIR_FILES)
