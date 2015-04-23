main: network.cpp main.cpp
	g++ ./network.cpp ./main.cpp -o ./main -std=c++11 `Magick++-config --cppflags --ldflags`
clean:
	rm -f ./main ./network.o ./main.o
