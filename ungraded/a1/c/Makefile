all:
	gcc -c -g -Os -march=i686 -m32 -ffreestanding -Wall -Werror src/test.c -o obj/test.o
	ld -m elf_i386 -static -Tlinker_scripts/test.ld -nostdlib --nmagic -o elf/test.elf obj/test.o
	objcopy -O binary elf/test.elf bin/test.bin
	dd if=bin/test.bin of=img/usb.img
# gens:
# 	gcc -S -Os -march=i686 -m16 -ffreestanding -Wall -Werror src/test.c -o asm/test.s
# 	as asm/test.s -o obj/test.o
# 	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
# 	dd if=bin/test.bin of=img/usb.img
