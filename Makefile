build:
	gcc -Wall -std=c99 ./src/*.c  -lmingw32 -lSDL2main -lSDL2 -Iinclude/SDL2 -o renderer.exe
run:
	./renderer.exe
clean:
	del renderer.exe	