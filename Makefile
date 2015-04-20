#Makefile for rshell
opt := -Wall -Werror -ansi -pedantic
comp := g++

all: rshell

rshell:
	mkdir bin
	$(comp) $(opt) ./src/main.cpp -o ./bin/rshell
