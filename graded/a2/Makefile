CFLAGS = -O0 -pthread -lm -g -Wall -Werror -std=c99

N ?= 1000000
t ?= 10

all: bin/main
	bin/main $(N) $(t)

bin/main: obj/main.o obj/vector.o
	gcc -o $@ $^ $(CFLAGS)

obj/main.o: src/main.c
	gcc -c -o $@ $^ $(CFLAGS)

obj/vector.o: src/vector.c
	gcc -c -o $@ $^ $(CFLAGS)

clean:
	rm -f obj/*
	rm -f bin/*
