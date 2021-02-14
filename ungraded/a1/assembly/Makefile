AS=/bin/as
LD=/bin/ld
LDFLAGS=--oformat=binary -Ttext 0x7c00

GENERATED_FILES=img/usb.img obj/test.o elf/test.elf bin/test.bin boot/bootloader.vfd

img/usb.img: bin/test.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$^ of=$@

boot/bootloader.vfd: bin/test.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd status=noxfer conv=notrunc if=$^ of=$@

bin/test.bin: obj/test.o
	$(LD) $(LDFLAGS) $^ -o $@

obj/test.o: src/test.S
	$(AS) $^ -o $@

clean:
	rm $(GENERATED_FILES)

# bochs:
# 	as src/test.S -o obj/test.o
# 	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
# 	dd if=/dev/zero of=img/usb.img bs=512 count=2880
# 	dd if=bin/test.bin of=img/usb.img
# 
# virtual:
# 	as src/test.S -o obj/test.o
# 	ld obj/test.o -o bin/test.bin --oformat=binary -Ttext 0x7c00
# 	head -c 1474560 /dev/zero > bootloader.vfd
# 	dd status=noxfer conv=notrunc if=bin/test.bin of=bootloader.vfd

