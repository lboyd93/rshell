# rshell

The purpose of this program is to execute a simple command shell named rshell which is very identical to the bash shell. 

*It first prints out a command prompt with the username and hostname. 

*It then reads in commands and executes them to an extent.

*Rshell works with comments in the command line and also multple flags.

## Add Ons

### ls

This program also contains an almost fully working implementation of ls. You can use the flags "-a", "-l", "-R", or any combination of the three on any directory on your hard drive. 

#BUGS

##rshell

This program has a few bugs. 

*It is not working when you input multiple commands. 

*Rshell only works when you have one command with flags, e.g. "ls -a" , "ls -a1", etc.

*It does not work with && or || or ; connectors for the command line.

##ls

This implementation of ls has a few known bugs.

* The combination of all three flags "-alR", "-laR", etc. is not working.

*When you want to use ls on a file, you get an error that you cannot open a directory.

*Flags: "-Rl", "-R", are not working.
