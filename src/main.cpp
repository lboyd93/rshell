#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[])
{	
	string input;
	cout << "$ ";	
	while(cin >> input ) {
		cout << "$ ";
		if(input == "exit")
			exit(1);
	}
	
	int i = fork();
	if(i==0) {
		if (-1==execvp("ls", argv)) {
			perror("execvp");
		}
	}
	else{
		cout << "HI!";
	}
	
	return 0;
}
