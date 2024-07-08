CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

vm: main.c
	$(CC) $(CFLAGS) -o vm main.c $(LIBS)
