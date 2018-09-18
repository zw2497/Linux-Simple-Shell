CC = gcc

w4118_sh: shell.c
	$(CC) -o w4118_sh shell.c -I.
clean:
	rm -f w4118_sh