all: basehangul.c han.c han.h
	gcc -o basehangul basehangul.c han.c han.h -Wall

.PHOHY: clean
clean:
	rm -f basehangul
