
.SUFFIXES:

.PHONY: all clean

all: myos.iso

SOURCE_DIR := src
BUILD_DIR  := build


CONTAINER_CMD := podman run -v "$(shell pwd)":"/scratch" \
                            --workdir="/scratch"         \
                            --network=none               \
                            -e TERM                      \
                            -t                           \
                            cc-i686:latest

CC := $(CONTAINER_CMD) i686-elf-gcc
LD := $(CONTAINER_CMD) i686-elf-ld
AS := $(CONTAINER_CMD) i686-elf-as
AR := $(CONTAINER_CMD) i686-elf-ar

C_SOURCES := $(shell find $(SOURCE_DIR) ! -name 'test_*' -name '*.c')
ASM_SOURCES := $(shell find $(SOURCE_DIR) -name '*.S')
OBJECTS := $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, $(C_SOURCES:.c=.o) $(ASM_SOURCES:.S=.o))
DEPENDS := $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, $(C_SOURCES:.c=.d))

CFLAGS := -MMD -ffreestanding -nostdlib -O1 -Wall -Wextra -Werror -std=c2x -I$(SOURCE_DIR)/include -no-pie -fstack-protector-strong -g3
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
ASFLAGS :=

#$(info C_SOURCES is $(C_SOURCES))
#$(info ASM_SOURCES is $(ASM_SOURCES))
#$(info OBJECTS is $(OBJECTS))
#$(info DEPENDS is $(DEPENDS))

run: myos.iso
	qemu-system-i386 -d int -no-reboot -cdrom myos.iso

cross-compiler: cross-compiler-image/Dockerfile
	podman build cross-compiler-image -t cc-i686

clean:
	rm -f $(BUILD_DIR) myos.iso

myos.iso: $(BUILD_DIR)/myos.bin
	@mkdir -p $(BUILD_DIR)/image/boot/grub
	cp -v $< $(BUILD_DIR)/image/boot/
	cp -v grub.cfg $(BUILD_DIR)/image/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(BUILD_DIR)/image

$(BUILD_DIR)/myos.bin: $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -T linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

-include $(DEPENDS)

$(BUILD_DIR)/kernel/interrupts.o: $(SOURCE_DIR)/kernel/interrupts.c Makefile
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) -mgeneral-regs-only -mno-red-zone $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c Makefile
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.S Makefile
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@


###################
#      TESTS      #
###################

TEST_SOURCE_DIR   := $(SOURCE_DIR)/test
TEST_BUILD_DIR   := $(BUILD_DIR)/test

TEST_SOURCES := $(shell find $(TEST_SOURCE_DIR) -name 'test_*.c')
TEST_DEPENDS := $(patsubst $(TEST_SOURCE_DIR)/%, $(TEST_BUILD_DIR)/%, $(TEST_SOURCES:.c=.d))
TEST_OUTPUT  := $(patsubst $(TEST_SOURCE_DIR)/%, $(TEST_BUILD_DIR)/%, $(TEST_SOURCES:.c=))

$(info TEST_SOURCES is $(TEST_SOURCES))
$(info TEST_OUTPUT is $(TEST_OUTPUT))

tests: $(TEST_OUTPUT)

-include $(TEST_DEPENDS)

$(TEST_BUILD_DIR)/test_%: $(TEST_SOURCE_DIR)/test_%.c $(SOURCE_DIR)/lib/%.c | Makefile
	@mkdir -p $(@D)
	gcc -O1 -fsanitize=address,undefined -Wall -Wextra -Werror -g3 -std=c2x -D_FORTIFY_SOURCE=2 -I$(SOURCE_DIR)/include -o $@ $^

