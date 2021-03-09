run: bin/single_thread
	bin/single_thread

bin/single_thread: tests/single_thread.c lib/libmyOwnthread.so
	gcc -g -o $@ $^ -L./lib -lmyOwnthread

obj/queue.o: src/queue.c
	gcc -Wall -fpic -g -o $@ -c $^

obj/myOwnthread.o: src/myOwnthread.c
	gcc -Wall -fpic -g -o $@ -c $^

lib/libmyOwnthread.so: obj/queue.o obj/myOwnthread.o
	gcc -g -o $@ $^ -shared

clean:
	rm -f lib/*
	rm -f obj/*
