
.PHONY: build clean

SRCS = src/main.cpp src/parser.cpp src/resource_man.cpp

build: main

main: $(SRCS)
	gcc -o main $< -lpthread

clean:
	rm -f main
