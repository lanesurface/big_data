CFLAGS:=-g -Wall -Wextra -Wno-unused-function
LDLIBS:=-lm

#hash: hash.c
#	$(CC) -g -Wall -Wextra -o $@ $^ -lm

hash: hmap.o main.o
	$(CC) -o $@ $? $(LDLIBS)

%.o: %.c 
	$(CC) -c $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf hash *.o *.dSYM
