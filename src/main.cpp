#include <iostream>
#include <string>
#include <stdlib.h>	//exit
#include <stdio.h>	//perror
#include <sys/wait.h>	//wait
#include <unistd.h>	//execvp, fork, gethost, getlogin
#include <string.h>	//strtok
using namespace std;

int main(int argc, char *argv[]){
	
	//getting login info
	char *login = getlogin();
	if(login == NULL){
		perror("get login");
	}
	
	string cmndline;
	//execute
	while(1){
		
		//string cmndline;
		cout << login << " $ ";
		//get the command line
		getline(cin, cmndline);
		
		//cout << cmndline;
			
		if(cmndline == "exit")
			break;
		
		
		//int i = fork();
		//if(i==0) {
		//	if (-1==execvp(argv[0], argv)) {
		//		perror("execvp");
		//	}
		//}
		//else{
		//	wait(0);
		//	cout << "HI!";
		//}
	}
	
	return 0;
}
