.POSIX:

CFLAGS := -Wall -Wextra -pedantic -Os -march=native
LIBS := `pkg-config --cflags --libs glew sdl2` -lm -I .

smart-rockets: src/nuklear.o src/*
	$(CC) $(CFLAGS) $(LIBS) src/main.c src/nuklear.o -o $@

src/nuklear.o: src/nuklear.c
	$(CC) $(CFLAGS) $(LIBS) -c src/nuklear.c -o src/nuklear.o

run: smart-rockets
	./smart-rockets

clean:
	rm -f smart-rockets src/nuklear.o

.PHONY: run clean prof

