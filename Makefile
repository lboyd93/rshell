#Makefile for rshell
opt := -Wall -Werror -ansi -pedantic
comp := g++

all: bin rshell ls

bin:
	[ ! -d $bin ] && mkdir bin
rshell: bin
	# mkdir bin
	$(comp) $(opt) ./src/rshell.cpp -o ./bin/rshell
ls: bin
	$(comp) $(opt) ./src/ls.cpp -o ./bin/ls
#clean
	#rm -rf bin
