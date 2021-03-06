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
	$(DEL) -f **/*.elf *.o **/*.o **/**/*.o iso/boot/*.elf *.img *.iso *.iso-2 *.elf

run: $(PROJECT).iso
	$(QEMU) -soundhw pcspk -serial mon:stdio -hda $< 

run-multi-1: $(PROJECT).iso
	$(QEMU) -soundhw pcspk -serial mon:stdio -serial telnet:localhost:1111,nowait -drive format=raw,file=$<

run-multi-2: $(PROJECT).iso
	cp $< $<-2
	$(QEMU) -soundhw pcspk -serial mon:stdio -serial telnet:localhost:2222,nowait -drive format=raw,file=$<-2

debug: kernel.elf
	$(QEMU) -soundhw pcspk -serial mon:stdio -s -S -kernel $< &
	gdb -iex "set auto-load safe-path .gdbinit" $<

zip:
	$(DEL) *.zip
	$(CD) .. && $(ZIP) -r $(ZIPFILE) $(LOCATION) $(ZIP_EXCLUDE) && $(MV) $(ZIPFILE) $(LOCATION)

