#include <iostream>
#include <string>
#include <stdlib.h>	//exit
#include <stdio.h>	//perror
#include <errno.h>	//perror
#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <unistd.h>	//execvp, fork, gethost, getlogin
#include <string.h>	//strtok
using namespace std;

int main(/*int argc, char *argv[]*/){	//can not figure out how to use argv
	
	char *login = getlogin();
	
	if(login == NULL){
		perror("get login");
	}
	//char host[100];
	
	//if(getlogin_r(login, sizeof(user)-1)){
	//	perror("Error with getlogin");
	//}
	
	
	//if(-1 == gethostname(host,sizeof(host)-1)){
        //	perror("Error with gethostname.");
	//}
	
	//string cmndline;
	//execute
	while(1){
		
		string input;
		char cmndline[1024];
		
		cout << login <<  "$ ";
		
		//get the input from user
		getline(cin, input);
		
		//char *cstr[input.length()+1];
		//change input from string to cstring
		strcpy(cmndline, input.c_str());
			
		if(!strcmp(cmndline, "exit"))
			exit(1);	
		
		//tokenize the commandline
		char *Tok;
		char delim[]= " ;&|";
		Tok=strtok(cmndline, delim);
			
		//cout << Tok ;
		//use argv to store the tokens in
			
		int i = fork();
		if(i==0) {
			//error check execvp
			if (-1==execvp(argv[0], argv)) {
				perror("execvp didn't work correctly");
			}
		}
		//in parent
		else{
			if(wait(0)==-1)
				perror("Error in wait");
			cout << "In parent waiting.";
		}
	}	
	return 0;
}
