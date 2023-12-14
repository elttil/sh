CFLAGS=-g -Wall -Wextra -pedantic -Werror
INCLUDE=-I.
OBJ=sh.o lexer.o ast.o test.o util.o

all: sh

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $<

sh: $(OBJ)
	$(CC) $^ -o $@

internaltest: sh
	ln -sf sh shelltest
	./shelltest

clean:
	rm shelltest sh $(OBJ)
