build:
	gcc -g -ggdb -Wall -std=c99 ./src/*.c  -lmingw32 -lSDL2main -lSDL2 -Iinclude/SDL2 -lm -o renderer.exe
run:
	./renderer.exe
clean:
	del renderer.exe
run_build:
	$(MAKE) build
	$(MAKE) run