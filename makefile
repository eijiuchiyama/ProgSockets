.PHONY: build clean

SRCS = src/main.cpp src/parser.cpp src/resource_man.cpp

build: main

main: $(SRCS)
	g++ -o main $(SRCS) -lpthread

clean:
	rm -f main
