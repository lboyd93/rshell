#Makefile for rshell
opt := -Wall -Werror -ansi -pedantic
comp := g++

all: bin rshell ls

bin:
	[ ! -d $bin ] && mkdir bin
rshell: bin
	$(comp) $(opt) ./src/rshell.cpp -o ./bin/rshell
ls: bin
	$(comp) $(opt) ./src/ls.cpp -o ./bin/ls
cp: bin
	$(comp) $(opt) ./src/cp.cpp -o ./bin/cp
clean:
	rm -rf bin
