CFLAGS = -O3 -pthread -lm -g -Wall -Werror -std=c99

bin/main: obj/main.o obj/vector.o
	gcc -o bin/main obj/vector.o obj/main.o $(CFLAGS)

obj/main.o: src/main.c
	gcc -c -o $@ $^ $(CFLAGS)

obj/main_static.o: src/main_static.c
	gcc -c -o $@ $^ $(CFLAGS)

obj/vector.o: src/vector.c
	gcc -c -o $@ $^ $(CFLAGS)

clean:
	rm -f obj/*
	rm -f bin/*
