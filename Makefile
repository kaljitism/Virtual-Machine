CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

all: evasm vmi

evasm: evasm.c vm.c
	$(CC) $(CFLAGS) -o evasm evasm.c $(LIBS)

vmi: vmi.c vm.c
	$(CC) $(CFLAGS) -o vmi vmi.c $(LIBS)
