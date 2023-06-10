CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -m64

all: cachesim

cachesim: cache.c cache.h cachesim.c 
	$(CC) $(CFLAGS) -o cachesim cache.c cachesim.c -lm 

clean:
	rm -rf *.o
	rm -f *.tar
	rm -f cachesim
