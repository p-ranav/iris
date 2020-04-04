all:
	g++ -I../../include -std=c++17 -pthread -o battery battery.cpp -lzmq
	g++ -I../../include -std=c++17 -pthread -o led led.cpp -lzmq

clean:
	rm -f battery led
