CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

.PHONY: all
all: evasm vmi

evasm: ./src/evasm.c ./src/vm.c
	$(CC) $(CFLAGS) -o evasm ./src/evasm.c $(LIBS)

vmi: ./src/vmi.c ./src/vm.c
	$(CC) $(CFLAGS) -o vmi ./src/vmi.c $(LIBS)

.PHONY: examples
examples: ./examples/fibonacci.vm ./examples/test.vm

./examples/fibonacci.vm: ./examples/fibonacci.evasm
	./evasm ./examples/fibonacci.evasm ./examples/fibonacci.vm:

./examples/test.vm: ./examples/test.evasm
	./evasm ./examples/test.evasm ./examples/test.vm:
