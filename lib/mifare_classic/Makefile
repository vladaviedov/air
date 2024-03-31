CC=../../build/gcc/bin/arm-linux-gnueabihf-g++
CFLAGS=-Wall -Wextra -g -std=c++20 -I ../build/include -I ./include
AR=../../build/gcc/arm-linux-gnueabihf/bin/ar
ARFLAGS=rsc

OUT=../../build/lib/libmifare_classic.a
BUILD_OBJ_DIR=../../build/obj/mifare_classic
BUILD_INC_DIR=../../build/include/nfc

.PHONY:
all: $(BUILD_OBJ_DIR) $(OUT) headers

$(OUT): $(BUILD_OBJ_DIR)/driver_mifare_classic.o
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_OBJ_DIR)/driver_mifare_classic.o: src/driver_mifare_classic.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_OBJ_DIR):
	mkdir -p $@

.PHONY: headers
headers:
	mkdir -p $(BUILD_INC_DIR)
	cp src/driver_mifare_classic.h $(BUILD_INC_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_OBJ_DIR)
	rm $(OUT)
