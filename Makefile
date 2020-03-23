CC = gcc
CFLAGS = -g -Wall
OBJMODULES = tree.o list.o exec.o
MAIN = part.c
PROG = task5

%.: %.c %.h 
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG): $(MAIN) $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

run:
	./$(PROG)
clean:
	-rm -f *.o $(PROG)
