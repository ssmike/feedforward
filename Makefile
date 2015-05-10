all: main init run

main: network.cpp main.cpp
	g++ -g ./network.cpp ./main.cpp -o ./main -std=c++11 `Magick++-config --cppflags --ldflags`

run: network.cpp run.cpp
	g++ -g ./network.cpp ./run.cpp -o ./run -std=c++11 `Magick++-config --cppflags --ldflags`

init: network.cpp init.cpp
	g++ -g ./network.cpp ./init.cpp -o ./init -std=c++11 `Magick++-config --cppflags --ldflags`

clean:
	rm -f ./main  ./init ./run 
