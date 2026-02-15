CC=gcc
CFLAGS=-Wall -Iinclude

SRC=src/main.c src/database.c src/pager.c

database:
	$(CC) $(CFLAGS) $(SRC) -o main

