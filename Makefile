all: shell

shell: shell.o
	g++ shell.o -o shell

shell.o: shell.cpp
	g++ -c -o shell.o shell.cpp

clean:
	rm -f shell.o shell

run: shell
	./shell

