PROJECT = x86-nibbles

SOURCES += src/main.c

ASSOURCES = boot/loader.S boot/isr.S

LDSCRIPT = linker.ld

LIBPATH = lib
include $(LIBPATH)/lib.mk

CC = gcc
AS = nasm
LD = ld
CP = cp
MV = mv
CD = cd
DEL = rm -rf
ZIP = zip
QEMU = qemu-system-i386

CFLAGS  += -nostdinc -nostdlib -nodefaultlibs -nostartfiles -static \
	   -ffreestanding -fno-builtin -fno-exceptions -fno-stack-protector \
	   -m32 -std=gnu99 -O0 -Wall -Wextra -g
ASFLAGS = -f elf32 -g -F dwarf -O0
LDFLAGS = -m elf_i386 -nostdlib

OBJECTS = $(SOURCES:.c=.o)
ASOBJECTS = $(ASSOURCES:.S=.o)

DATE=`date +"%Y%m%d"`
ZIPFILE=$(PROJECT)-$(DATE).zip
ZIP_EXCLUDE=-x '*.git*' -x '*.sw*' -x '*.o' -x '*.zip'
LOCATION=$(shell basename `pwd`)

all: $(PROJECT).elf

$(PROJECT).iso: $(PROJECT).elf
	$(CP) $< iso/boot/kernel.elf
	grub-mkrescue -o $@ iso

$(PROJECT).elf: $(ASOBJECTS) $(OBJECTS)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@ $^

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $(CCFLAGS) $<

clean:
	$(DEL) -f **/*.elf *.o **/*.o **/**/*.o iso/boot/*.elf *.img *.iso *.elf

run: $(PROJECT).iso
	$(QEMU) -soundhw pcspk -serial mon:stdio -hda $< 

debug: kernel.elf
	$(QEMU) -soundhw pcspk -serial mon:stdio -s -S -kernel $< &
	gdb -iex "set auto-load safe-path .gdbinit" $<

zip:
	$(DEL) *.zip
	$(CD) .. && $(ZIP) -r $(ZIPFILE) $(LOCATION) $(ZIP_EXCLUDE) && $(MV) $(ZIPFILE) $(LOCATION)

