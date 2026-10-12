CC = gcc
CFLAGS = -std=c17 -O1 -Wall -Wextra -Wpedantic -Werror
SANITIZE = -fsanitize=address,undefined

test: tests/test_aes
	./tests/test_aes

tests/test_aes: src/aes.c src/aes.h tests/test_aes.c
	$(CC) $(CFLAGS) $(SANITIZE) -Isrc src/aes.c tests/test_aes.c -o $@

asm: src/aes.c src/aes.h
	$(CC) -std=c17 -O2 -S src/aes.c -o aes-O2.s

clean:
	rm -f tests/test_aes aes-O2.s

.PHONY: test asm clean
