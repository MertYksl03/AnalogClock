CC = gcc
CFLAGS = -Wall -Werror -g -std=c99
INCLUDES = SDL2 


build:
	$(CC) $(CFLAGS) -o AnalogClock *c -lm -l$(INCLUDES)
run:
	make build && ./AnalogClock