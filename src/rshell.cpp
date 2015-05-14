#include <iostream>
#include <string>
#include <stdlib.h>	//exit
#include <stdio.h>	//perror
#include <errno.h>	//perror
#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <unistd.h>	//execvp, fork, gethost, getlogin
#include <string.h>	//strtok
#include <vector>
using namespace std;

//checks for comments and if stuff is after comments, it deletes it
void checkcomm(string &str){
	size_t comment = str.find("#");
	if(comment != string :: npos)
		str.erase(comment);
}

void forkPipes(char **argv, char **afterPipe);
void simpleFork(char **argv);
void getPipes(char** argv);

int main(/*int argc, char *argv[]*/){	//couldn't get argv to work
	
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
	
	//execute
	while(1){
		
		string input;
		char cmndline[1024];
		
		
		
		cout << login << "@" << host << "$ ";
		
		//get the input from user
		getline(cin, input);
		
		//check if user pushes enter 
		if(input == "")
			continue;
		
		//check for comments
		checkcomm(input);

		//check if user inputs exit	
		if( input.find("exit") != string::npos )
			exit(0);	
				

		//change input from string to cstring
		strcpy(cmndline, input.c_str());
		
		//tokenize the commandline
		char *tok; //*save;
		char delim[]= " ;&|";
		
		int size=input.size()+1;
		//vector<int> order;
		
		tok=strtok(cmndline, delim);
			
		char **argv=new char*[size];
		
		int pos=0;
		while(tok != NULL)
		{
			argv[pos]=tok;
			pos++;
			tok = strtok(NULL,delim);
		}
		argv[pos]=NULL;
		
		
		//for(unsigned i=0; i < order.size(); i++)
		//	cout << order[i] << ' ';
		//simpleFork(argv);
		
		getPipes(argv);
		
		delete argv;	
	}
	
	return 0;
}

void simpleFork(char **argv){
	int status = 0;
	int i = fork();
	if(i == -1){
		//error check fork
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
		//error check parent
		if(wait(&status)==-1){
			perror("Error in wait");
			exit(1);
		}
	}
}

void getPipes(char** argv){
	bool pipeFound = false;
	
	char **args = argv;
	char **nextA = argv;
	
	for(int i=0; argv[i] != '\0'; i++){
		if(strcmp(argv[i], "|") == 0){
			pipeFound=true;
			argv[i] = '\0';
			
			char** beforeA = args;
			nextA = args + i + 1;
			forkPipes(beforeA, nextA);
		}
	}
	
	if(!pipeFound){
		//TODO
	}
}

void forkPipes(char **argv, char **afterPipe){
	int fd[2];
	int pid;
	int status;
	if(pipe(fd) == -1){
		perror("Error with pipe.");
		exit(EXIT_FAILURE);
	}
	
	pid = fork();
	if(pid == -1){
		perror("There was an error with piping fork.");
		exit(1);
	}
	else if(pid ==0){
		if(close(fd[0]) == -1){
			perror("Error with close in piping.");
			exit(1);
		}
		if(dup2(fd[1],1)==-1){
			perror("Error with dup2 in piping.");
		}
		if(execvp(argv[0], argv) == -1){
			perror("There was an error with execvp piping");
			exit(1);
		}
		else{
			exit(0);
		}	
	}
	else{
		if(close(fd[0]) == -1){
			perror("Error with close in piping.");
			exit(1);
		}
		
		int fd0 = dup(0);
		if(fd0 == -1){
			perror("There was an error with dup.");
		}
		if(dup2(fd[0],0) == -1){
			perror("Error with dup2 in piping.");
		}
		while(waitpid(-1, &status, 0) >= 0);
		getPipes(afterPipe);
		if(dup2(fd0,0)==-1){
			perror("Error with dup2 in piping.");
		}
		
	}	
}
