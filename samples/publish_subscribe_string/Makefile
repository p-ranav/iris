all:
	g++ -I../../include -std=c++17 -pthread -o publisher publisher.cpp -lzmq
	g++ -I../../include -std=c++17 -pthread -o subscriber subscriber.cpp -lzmq

clean:
	rm -f publisher subscriber
