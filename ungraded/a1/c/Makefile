CC=/bin/gcc
LD=/bin/ld
ARCH=i686
CFLAGS=-g -Os -march=$(ARCH) -m16 -ffreestanding -Wall -Werror
LDSCRIPT=linker_scripts/test.ld
LDFLAGS=-m elf_i386 -static -T$(LDSCRIPT) -nostdlib --nmagic

GENERATED_FILES=img/usb.img obj/test.o elf/test.elf bin/test.bin boot/bootloader.vfd

.PHONY: run
run: img/usb.img boot/bootloader.vfd

img/usb.img: bin/test.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$^ of=$@

boot/bootloader.vfd: bin/test.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd status=noxfer conv=notrunc if=$^ of=$@

obj/test.o: src/test.c
	$(CC) -c $(CFLAGS) $^ -o $@

elf/test.elf: obj/test.o
	$(LD) $(LDFLAGS) $^ -o $@

bin/test.bin: elf/test.elf
	objcopy -O binary $^ $@

clean:
	rm $(GENERATED_FILES)

# gens:
# 	gcc -S -Os -march=i686 -m16 -ffreestanding -Wall -Werror src/test.c -o asm/test.s
# 	as asm/test.s -o obj/test.o
# 	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
# 	dd if=bin/test.bin of=img/usb.img
