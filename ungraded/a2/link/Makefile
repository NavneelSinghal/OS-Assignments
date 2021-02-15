FILES= my_math/libmy_math.so main.o main

all: $(FILES)

run: main
	LD_LIBRARY_PATH=./my_math ./main

clean:
	rm -f $(FILES)

my_math/libmy_math.so: my_math/my_math.c
	gcc -o my_math/libmy_math.so -fpic -shared my_math/my_math.c

main: main.o
	gcc -o main main.o -lmy_math -L./my_math/
	# nm -D main
	# ldd main

main.o: main.c
	gcc -c -o main.o main.c
	# readelf -h main.o
	# readelf -S main.o
	# readelf -l main.o
	# nm main.o

