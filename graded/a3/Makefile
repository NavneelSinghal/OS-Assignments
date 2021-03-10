runlockyield: bin/lockyield
	bin/lockyield

runlock: bin/lock
	bin/lock

runrecyield: bin/recyield
	bin/recyield

runyield: bin/yield
	bin/yield

runmultiple: bin/multiple_threads
	bin/multiple_threads

runsingle: bin/single_thread
	bin/single_thread

bin/lockyield: tests/lock_yield.c lib/libmyOwnthread.so
	gcc -O0 -g -o $@ $^ -L./lib -lmyOwnthread

bin/lock: tests/lock.c lib/libmyOwnthread.so
	gcc -O0 -g -o $@ $^ -L./lib -lmyOwnthread

bin/recyield: tests/yield_nested.c lib/libmyOwnthread.so
	gcc -O0 -g -o $@ $^ -L./lib -lmyOwnthread

bin/yield: tests/yield.c lib/libmyOwnthread.so
	gcc -O0 -g -o $@ $^ -L./lib -lmyOwnthread

bin/multiple_threads: tests/multi_threads.c lib/libmyOwnthread.so
	gcc -O0 -g -o $@ $^ -L./lib -lmyOwnthread

bin/single_thread: tests/single_thread.c lib/libmyOwnthread.so
	gcc -O0 -g -o $@ $^ -L./lib -lmyOwnthread

obj/queue.o: src/queue.c
	gcc -O0 -Wall -fpic -g -o $@ -c $^

obj/myOwnthread.o: src/myOwnthread.c
	gcc -O0 -Wall -fpic -g -o $@ -c $^

lib/libmyOwnthread.so: obj/queue.o obj/myOwnthread.o
	gcc -O0 -g -o $@ $^ -shared

clean:
	rm -f lib/*
	rm -f obj/*
	rm -f bin/*
