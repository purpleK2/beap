CC = gcc

.PHONY: all clean test

all: test

test:
	$(CC) -g -O0 -o test.out test/main.c beap/beap.c beap/tlsf.c

clean:
	rm -f test.out