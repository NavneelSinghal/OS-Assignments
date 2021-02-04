bochs:
	as src/test.S -o obj/test.o
	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
	dd if=/dev/zero of=img/usb.img bs=512 count=2880
	dd if=bin/test.bin of=img/usb.img

virtual:
	as src/test.S -o obj/test.o
	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
	head -c 1474560 /dev/zero > bootloader.vfd
	dd status=noxfer conv=notrunc if=bin/test.bin of=bootloader.vfd

