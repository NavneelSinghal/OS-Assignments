CFLAGS = -O3 -pthread -lm -g -Wall -Werror -std=c99

all: bin/main

bin/main: obj/main.o obj/vector.o
	gcc -o $@ $^ $(CFLAGS)

obj/main.o: src/main.c
	gcc -c -o $@ $^ $(CFLAGS)

obj/vector.o: src/vector.c
	gcc -c -o $@ $^ $(CFLAGS)

clean:
	rm -f obj/*
	rm -f bin/*
