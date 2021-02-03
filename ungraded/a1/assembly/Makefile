all:
	as src/test.S -o obj/test.o
	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
	dd if=bin/test.bin of=img/usb.img
