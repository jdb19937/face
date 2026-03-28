CC      ?= cc
CFLAGS  ?= -Wall -Wextra -pedantic -std=c99 -O2

omnia: face

face: face.c
	$(CC) $(CFLAGS) -o $@ $<

purga:
	rm -f face

.PHONY: omnia purga
