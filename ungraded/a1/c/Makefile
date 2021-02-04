bochs:
	gcc -c -g -Os -march=i686 -m16 -ffreestanding -Wall -Werror src/test.c -o obj/test.o
	ld -m elf_i386 -static -Tlinker_scripts/test.ld -nostdlib --nmagic -o elf/test.elf obj/test.o
	objcopy -O binary elf/test.elf bin/test.bin
	dd if=/dev/zero of=img/usb.img bs=512 count=2880
	dd if=bin/test.bin of=img/usb.img

virtual:
	gcc -c -g -Os -march=i686 -m16 -ffreestanding -Wall -Werror src/test.c -o obj/test.o
	ld -m elf_i386 -static -Tlinker_scripts/test.ld -nostdlib --nmagic -o elf/test.elf obj/test.o
	objcopy -O binary elf/test.elf bin/test.bin
	head -c 1474560 /dev/zero > bootloader.vfd
	dd status=noxfer conv=notrunc if=bin/test.bin of=bootloader.vfd

# gens:
# 	gcc -S -Os -march=i686 -m16 -ffreestanding -Wall -Werror src/test.c -o asm/test.s
# 	as asm/test.s -o obj/test.o
# 	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
# 	dd if=bin/test.bin of=img/usb.img
