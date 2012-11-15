all: hello

hello: hello.o util.o cache_util.o 
	gcc -Wall `pkg-config fuse --cflags --libs` $^ -o hello

%.o: %.c
	gcc -c $^

hello.o: hello.c
	gcc -Wall `pkg-config fuse --cflags --libs` -c hello.c 

util.o: util.c
	gcc -c $^

cache_util.o: cache_util.c
	gcc -c $^

#list.o: list.c
#	gcc -c $^

clean:
	rm -f *.o hello *~
