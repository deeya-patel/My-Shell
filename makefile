CC= gcc
CFLAGS = -g -Wall -fsanitize=address

mysh.o: mysh.c
	$(CC) $(CFLAGS) mysh.c -o mysh
clean:
	rm -f mysh.o