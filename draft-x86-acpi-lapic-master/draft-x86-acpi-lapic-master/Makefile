
CFLAGS	:=	-m32 -Wall -Wextra -pedantic -Iinclude -fno-builtin -nostartfiles \
			-nostdlib

ASFLAGS	:=	-m16 -Wall -Wextra -pedantic

OBJECTS	:=	acpi.o common.o smp.o
TARGET	:=	kernel.bin

all: $(TARGET)

$(TARGET): boot.o $(OBJECTS)
	$(LD) acpi.o common.o smp.o -melf_i386 --script=kernel.ld -o kernel.elf
	objcopy -O binary -j .text boot.o boot.bin
	cat boot.bin kernel.elf > $@

clean:
	-$(RM) $(TARGET) $(OBJECTS) kernel.elf boot.bin boot.o
