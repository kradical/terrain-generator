renderer: *.cpp *.c
	g++ -o renderer.out *.cpp *.c -I include -std=c++11 -lSDL2main -lSDL2
