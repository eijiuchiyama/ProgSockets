
.PHONY: build clean

SRCS = main.c parser.c resource_man.c

build: main

main: $(SRCS)
	gcc -o main $< -lpthread

clean:
	rm -f main