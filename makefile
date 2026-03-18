
.PHONY: build clean

SRCS = src/main.c src/parser.c src/resource_man.c

build: main

main: $(SRCS)
	gcc -o main $< -lpthread

clean:
	rm -f main