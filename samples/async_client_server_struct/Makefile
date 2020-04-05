all:
	g++ -I../../include -std=c++17 -pthread -o client client.cpp -lzmq
	g++ -I../../include -std=c++17 -pthread -o broker broker.cpp -lzmq
	g++ -I../../include -std=c++17 -pthread -o server server.cpp -lzmq

clean:
	rm -f client server
