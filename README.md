# rshell #

The purpose of this program is to execute a simple command shell named rshell which is very identical to the bash shell. 

* It first prints out a command prompt with the username and hostname. 

* It then reads in commands and executes them to an extent.

* Rshell works with comments in the command line and also multple flags.

* It is capable of I/O redirection using >>, >, and <.

* Rshell works with piping two or more  programs together.

## Add Ons ##

### ls ###

This program also contains an almost fully working implementation of ls comparable to bash ls. You can use the flags "-a", "-l", "-R", or any combination of the three on any directory on your hard drive. 

### cp ###

Rshell contains a fully working implementation of cp comparable to bash cp. It works the same as cp where there are two arguments needed and one optional argument. There are three different implementations of cp in the cp.cpp file. With the thrid argument, you can choose if you want to run all three and output their runtimes or run the fastest.

### rm ###

Rshell contains a fully working implementation of rm comparable to bash rm. It removes as many files as you would like. It can also remove directories if the -r flag is added to the command line.

### mv ###

Rshell contains a fully working implementation of mv that is comparable to bash mv. It takes as many files and/or directories and moves them to the last directory in your command line. With mv you can also rename a file.

#BUGS#

##rshell##

This program has a few bugs. 

* It is not working when you input multiple commands. 

* Rshell only works when you have one command with flags, e.g. "ls -a" , "ls -a1", etc.

* It does not work with && or || or ; connectors for the command line.

### I/O Redirection ###

* When using input redirection, there must be one space in between all your arguments.
ex ls >> ls.txt, ls > ls.txt

* When using piping, you cannot put more than 2 pipes together. When you have more than 2 pipes, the rest of the arguments are just ignored.

##ls##

This implementation of ls has a few known bugs.

* The combination of all three flags "-alR", "-laR", etc. is not working.

* When you want to use ls on a file, you get an error that you cannot open a directory.

* Flags: "-Rl", "-R", are not working.

* When outputting, the colunms in -l do not match the column with the longest character count.

* When outputting -l, my ls does not get the the reading permission of groups, users, or everyone.

* When outputting -a and -R together, it does not print out the "." files and their files as well.

* When you output -a with multiple directories, it does not output the name of the root before printing out all the file names. 
