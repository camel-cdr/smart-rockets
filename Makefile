CFLAGS := -Wall -Wextra -pedantic -O3
LIBS := `pkg-config --cflags --libs glew sdl2` -lm -I .

smart_rockets: src/*
	$(CC) $(CFLAGS) $(LIBS) src/main.c -o $@

run: smart_rockets
	./$<

clean:
	rm -f smart_rockets

.PHONY: run clean prof

