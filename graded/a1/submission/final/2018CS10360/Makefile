CC=/bin/gcc
CFLAGS=-O3 -g

TO_CLEAN=obj/mySyscallTest.o bin/mySyscallTest

.PHONY: all

all: bin/mySyscallTest

clean:
	rm -f $(TO_CLEAN)

obj/mySyscallTest.o: src/mySyscallTest.c
	$(CC) $(CFLAGS) -c -o $@ $^

bin/mySyscallTest: obj/mySyscallTest.o
	$(CC) $(CFLAGS) -o $@ $^
