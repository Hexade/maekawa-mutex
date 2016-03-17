all: setup client server

client: setup bin/client bin/client.config bin/quorum.config bin/server.config

server: setup bin/server bin/server.config

bin/client: obj/exception.o obj/utils.o obj/config.o obj/tcp_socket.o obj/connection_manager.o obj/sock_data_cb.o obj/registry.o obj/tcp_server.o obj/client.o
	g++ -Wall -pthread -std=c++11 -o bin/client obj/exception.o obj/utils.o obj/config.o obj/tcp_socket.o obj/connection_manager.o obj/sock_data_cb.o obj/tcp_server.o obj/client.o

bin/server: obj/exception.o obj/utils.o obj/config.o obj/tcp_socket.o obj/connection_manager.o obj/sock_data_cb.o obj/registry.o obj/tcp_server.o obj/server.o
	g++ -Wall -std=c++11 -o bin/server obj/exception.o obj/utils.o obj/config.o obj/tcp_socket.o obj/connection_manager.o obj/sock_data_cb.o obj/registry.o obj/tcp_server.o obj/server.o
    	
obj/client.o: src/client.cpp
	g++ -Wall -pthread -std=c++11 -c src/client.cpp -o obj/client.o

obj/server.o: src/server.cpp
	g++ -Wall -std=c++11 -c src/server.cpp -o obj/server.o

obj/tcp_server.o: src/tcp_server.h src/tcp_server.cpp
	g++ -Wall -c src/tcp_server.cpp -o obj/tcp_server.o

obj/registry.o: src/registry.h src/registry.cpp
	g++ -Wall -c src/registry.cpp -o obj/registry.o

obj/sock_data_cb.o: src/sock_data_cb.h src/sock_data_cb.cpp
	g++ -Wall -c src/sock_data_cb.cpp -o obj/sock_data_cb.o

obj/connection_manager.o: src/connection_manager.h src/connection_manager.cpp
	g++ -Wall -std=c++11 -c src/connection_manager.cpp -o obj/connection_manager.o

obj/tcp_socket.o: src/tcp_socket.h src/tcp_socket.cpp
	g++ -Wall -c src/tcp_socket.cpp -o obj/tcp_socket.o

obj/config.o: src/config.h src/config.cpp
	g++ -Wall -c src/config.cpp -o obj/config.o

obj/exception.o: src/exception.h src/exception.cpp
	g++ -Wall -c src/exception.cpp -o obj/exception.o

obj/utils.o: src/utils.h src/utils.cpp
	g++ -Wall -c src/utils.cpp -o obj/utils.o

bin/server.config:
	cp server.config bin/

bin/client.config:
	cp client.config bin/

bin/quorum.config:
	cp quorum.config bin/

setup: bin/ obj/

bin/:
	mkdir -p bin

obj/:
	mkdir -p obj

.PHONY: clean
clean:
	rm -rf bin/ obj/
