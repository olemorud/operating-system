
.SUFFIXES:

.PHONY: all clean

SOURCE_DIR := src
BUILD_DIR  := build

CONTAINER_CMD := podman run -v "$(shell pwd)":"/scratch"	\
						    --workdir="/scratch" 			\
							-e TERM							\
							-t								\
							cc-i686:latest

CC := $(CONTAINER_CMD) i686-elf-gcc
LD := $(CONTAINER_CMD) i686-elf-ld
AS := $(CONTAINER_CMD) i686-elf-as
AR := $(CONTAINER_CMD) i686-elf-ar

C_SOURCES := $(shell find $(SOURCE_DIR) -name '*.c')
ASM_SOURCES := $(shell find $(SOURCE_DIR) -name '*.S')
OBJECTS := $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, $(C_SOURCES:.c=.o) $(ASM_SOURCES:.S=.o))
DEPENDS := $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, $(C_SOURCES:.c=.d))

CFLAGS := -MMD -ffreestanding -O0 -Wall -Wextra -Werror -std=c2x -I$(SOURCE_DIR)/include -no-pie -fstack-protector-strong
ASFLAGS :=

$(info C_SOURCES is $(C_SOURCES))
$(info ASM_SOURCES is $(ASM_SOURCES))
$(info OBJECTS is $(OBJECTS))
$(info DEPENDS is $(DEPENDS))

all: myos.iso

run: myos.iso
	qemu-system-i386 -cdrom myos.iso

cross-compiler: cross-compiler-image/Dockerfile
	podman build cross-compiler-image -t cc-i686

myos.iso: $(BUILD_DIR)/myos.bin
	@mkdir -p $(BUILD_DIR)/image/boot/grub
	cp -v $< $(BUILD_DIR)/image/boot/
	cp -v grub.cfg $(BUILD_DIR)/image/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(BUILD_DIR)/image

$(BUILD_DIR)/myos.bin: $(OBJECTS)
	mkdir -p $(BUILD_DIR)
	$(CC) -T linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

-include $(DEPENDS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c Makefile
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.S Makefile
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@
