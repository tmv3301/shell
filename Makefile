all: shell

shell: shell.o
	g++ shell.o -o shell

shell.o: shell.c
	g++ -Wall -g -c shell.c

clean:
	rm -f shell.o shell

run: shell
	./shell

