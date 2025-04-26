CC = gcc
CFLAGS = -g \
		-O2 \
		-I beap

.PHONY: all clean test

all: test

test:
	$(CC) $(CFLAGS) -o beap_test test/main.c beap/beap.c beap/tlsf.c -D BEAP_DEBUG

clean:
	rm -f beap_test
