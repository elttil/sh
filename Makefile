CFLAGS=-g -Wall -Wextra -pedantic -Werror
INCLUDE=-I.
OBJ=sh.o lexer.o ast.o

all: sh

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $<

sh: $(OBJ)
	$(CC) $^ -o $@

clean:
	rm sh $(OBJ)
