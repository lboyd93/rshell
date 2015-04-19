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

//checks for comments and if stuff is after comments, it deletes it
void checkcomm(string &str){
	char comment='#';
	for(unsigned pos=0; pos < str.size(); pos++){
		if(str[pos] == comment){
			for(unsigned i=pos; i < str.size(); i++){
				str.erase(pos,str.size()-1);
			}
		}
	}
}

int main(/*int argc, char *argv[]*/){
	
	//char connector[3] = {'&', '|', ';'}; 	
	//get username and hostname
	char login[100];
	
	if(getlogin_r(login, sizeof(login)-1)){
		perror("Error with getlogin_r");
	}
	
	char host[100];
	
	if(-1 == gethostname(host,sizeof(host)-1)){
        	perror("Error with gethostname.");
	}
	
	for(int pos=0; pos<30; pos++){
		if(host[pos] == '.'){
			host[pos] = '\0';
		}
	}
	
	//string cmndline;
	//execute
	while(1){
		
		string input;
		char cmndline[1024];
		
		cout << login << "@" << host << "$ ";
		
		//get the input from user
		getline(cin, input);
		
		//check if user pushes enter or comment	
		if(input == "" || input == "#")
			continue;
		
		//check if user inputs exit	
		if( input.find("exit") != string::npos )
			exit(0);	
				
		//check for comments
		checkcomm(input);

		//change input from string to cstring
		strcpy(cmndline, input.c_str());
		
		//tokenize the commandline
		char *Tok;
		char delim[]= " ;&|";
		Tok=strtok(cmndline, delim);
			
		//cout << Tok ;
		
		int size=input.size()+1;
		char **argv=new char*[size];
		
		int pos=0;
		while(Tok != NULL)
		{
			argv[pos]=Tok;
			pos++;
			Tok = strtok(NULL,delim);
		}
		argv[pos]=NULL;
		
		//for(int i=0; i<size; i++)
		//	cout << argv[i] << " ";
		
		int status = 0;
		int i = fork();
		if(i == -1){
			perror("There was an error with fork()");
			exit(1);
		}
		else if(i == 0) {
			//error check execvp
			if (-1==execvp(argv[0], argv)) {
				perror("execvp didn't work correctly");
				exit(1);
			}
		}
		//in parent
		else if(i > 0){
			if(wait(&status)==-1){
				perror("Error in wait");
				return -1;
			}
			if(WEXITSTATUS(status) == 3){
				//delete argv;
				return -1;
			}
		}
	}
	//delete argv;
	
	return 0;
}
