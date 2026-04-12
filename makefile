.PHONY: build clean

SRCS = src/main.cpp src/parser.cpp src/resource_man.cpp src/handler.cpp src/logger.cpp

build: server client

server: $(SRCS)
	g++ -o server $(SRCS) -lpthread
	
client: 
	g++ -o client tcp-echo-client.cpp -lpthread

clean:
	rm -f server client
