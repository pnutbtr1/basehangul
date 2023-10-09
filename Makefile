CC = gcc
CFLAGS = -Wall -O2
PREFIX ?= /usr/local
DESTDIR ?= 

all: basehangul
basehangul: basehangul.c han.c han.h
	$(CC) $(CFLAGS) -o basehangul basehangul.c han.c han.h

han.c han.h: gen_han 상용한글2350자.txt
	./gen_han 상용한글2350자.txt

gen_han: gen_han.c
	$(CC) $(CFLAGS) -o $@ $^

.PHOHY: all clean install uninstall
clean:
	rm -f basehangul
	rm -f gen_han
	rm -f han.c han.h

install: basehangul
	install -D -m 755 basehangul $(DESTDIR)$(PREFIX)/bin/basehangul

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/basehangul
