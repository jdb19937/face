%: a.out
	./a.out $(MAKECMDGOALS)

a.out:
	cc face.c

.PHONY: default
