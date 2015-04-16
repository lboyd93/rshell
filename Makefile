#Makefile for rshell
opt := -Wall -Werror -ansi -pedantic
comp := g++

all: bin rshell

rshell:
	$(comp) $(opt) ./src/main.cpp -o ./bin/rshell
