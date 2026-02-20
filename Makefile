CC = gcc
CFLAGS = -Wall -Werror -g -std=c99
INCLUDES = SDL2 
SOURCES = src/*.c
OUT = AnalogClock

build:
	$(CC) $(CFLAGS) -o $(OUT) $(SOURCES) -lm -l$(INCLUDES)
run:
	make build && ./$(OUT)