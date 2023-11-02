all:
	g++ -Isrc/include -Lsrc/lib -o run main3.cpp -lmingw32 -lSDL2main -lSDL2 -fopenmp