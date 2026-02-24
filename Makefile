CC = gcc
CFLAGS = -Wall -Werror -g -std=c99
LIBS = -lSDL2 -lSDL2_ttf -lm
SOURCES = src/*.c
OUT = AnalogClock

build:
	$(CC) $(CFLAGS) -o $(OUT) $(SOURCES) $(LIBS)

run:
	make build && ./$(OUT)