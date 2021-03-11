CFLAGS=-O0 -fstack-protector-all

THREADS=1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40

lib/libmyOwnthread.so: obj/queue.o obj/myOwnthread.o
	@gcc $(CFLAGS) -g -o $@ $^ -shared

matrix_mult: bin/matrixmult bin/measurecontext
	@rm -f out/stats
	@touch out/stats
	@bash scripts/run.sh
	@echo "Total time plot saved in out/graph.png"
	@echo "Matrix context switch time plot saved in out/matrix_mult_context.png"
	@echo "Vanilla context switch time plot saved in out/context.png"

boundedBuffer: bin/boundedbuffer
	@bin/boundedbuffer 3 2 > out/boundedbufferstats
	@echo "Bounded buffer stats stored in out/boundedbufferstats"

all: matrix_mult boundedBuffer

runcvyield: bin/cvyield
	@bin/cvyield

runcv: bin/cv
	@bin/cv

runlockyield: bin/lockyield
	@bin/lockyield

runlock: bin/lock
	@bin/lock

runrecyield: bin/recyield
	@bin/recyield

runyield: bin/yield
	@bin/yield

runmultiple: bin/multiple_threads
	@bin/multiple_threads

runsingle: bin/single_thread
	@bin/single_thread

bin/boundedbuffer: target_tests/boundedbuffer.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -pthread -g -o $@ $^ -L./lib -lmyOwnthread

bin/measurecontext: target_tests/context.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -pthread -g -o $@ $^ -L./lib -lmyOwnthread

bin/matrixmult: target_tests/matrixmult.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -pthread -g -o $@ $^ -L./lib -lmyOwnthread

bin/cvyield: tests/cv_yield.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

bin/cv: tests/cv.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

bin/lockyield: tests/lock_yield.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

bin/lock: tests/lock.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

bin/recyield: tests/yield_nested.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

bin/yield: tests/yield.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

bin/multiple_threads: tests/multi_threads.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

bin/single_thread: tests/single_thread.c lib/libmyOwnthread.so
	@gcc $(CFLAGS) -g -o $@ $^ -L./lib -lmyOwnthread

obj/queue.o: src/queue.c
	@gcc $(CFLAGS) -Wall -fpic -g -o $@ -c $^

obj/myOwnthread.o: src/myOwnthread.c
	@gcc $(CFLAGS) -Wall -fpic -g -o $@ -c $^

clean:
	@rm -f lib/*
	@rm -f obj/*
	@rm -f bin/*
