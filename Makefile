CFLAGS:=-g -Wall -Wextra -Wno-unused-function
LDLIBS:=-lm

#hash: hash.c
#	$(CC) -g -Wall -Wextra -o $@ $^ -lm

hash: hash.o
	$(CC) -o $@ $? $(LDFLAGS)

%.o: %.c 
	$(CC) -c $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf hash *.o *.dSYM
