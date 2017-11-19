all: traffic_generator simulator
CC = g++ -std=c++11  -Wall

traffic_generator: traffic_generator.cpp
	$(CC) -o traffic_generator traffic_generator.cpp
simulator: simulator.cpp
	$(CC) -o simulator simulator.cpp
clean:
	rm traffic_generator simulator
