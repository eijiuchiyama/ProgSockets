
.PHONY: build clean

build: main

main: main.c
	gcc -o main main.c -lpthread

clean:
	rm -f main