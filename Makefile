CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

ifdef USE_SANITIZERS
CFLAGS += -fsanitize=undefined,leak,address
endif

all: raon.c
	$(CC) $(CFLAGS) $(SANITIZERS) $^


