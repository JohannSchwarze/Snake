all: main


clean:
	rm main


main: main.cpp
	g++ main.cpp -lncurses -o main

