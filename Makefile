all: basehangul
	tail -n +2 basehangul > basehangul.c
	gcc -o bh basehangul.c -Wall

.PHOHY: clean
clean:
	rm -f bh basehangul.c
