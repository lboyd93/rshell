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
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

//checks for comments and if stuff is after comments, it deletes it
void checkcomm(string &str){
	size_t comment = str.find("#");
	//cout << "THis is pos of comment: " << comment << endl;
	str[comment] = '\0';
	if(comment != string :: npos)
		str.erase(str.begin()+comment, str.end());
	//cout << "This is str: " << str << endl; 
}

void fixSpaces(string &input);
void IOredir(char **argv);
void forkPipes(char **argv, char **afterPipe);
void simpleFork(char **argv);
void getPipes(char** argv);

int main(){
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
		char buf[1000];
		int sdi = dup(0), sdo = dup(1);
		if(sdi == -1)
			perror("There was an error with dup");
		if(sdo == -1)
			perror("There was an error with dup");	
		cout << login << "@" << host << "$ ";
		
		//get the input from user
		getline(cin, input);
		
		//check if user pushes enter 
		if(input == "")
			continue;
		
		//checks the spacing
		fixSpaces(input);
			
		//check for comments
		checkcomm(input);

		strcpy(buf, input.c_str());
		
		//check if user inputs exit	
		if( input.find("exit") != string::npos )
			exit(1);	
		
		//tokenize the commandline
		char delim[]= " \t\n";
		
		int size=input.size()+1;
		
		char **argv=new char*[size];
		int pos=0;
		argv[pos]=strtok(buf, delim);
			
		while(argv[pos] != NULL)
		{
			pos++;
			argv[pos] = strtok(NULL,delim);
		}
		//argv[pos]=NULL;
		
		
		//for(unsigned i=0; argv[i] != NULL ; i++)
		//	cout << argv[i] << ' ';
		
		IOredir(argv);
		getPipes(argv);
		
		if(dup2(sdi, 0) == -1)
			perror("There was an error with dup2");
		if(dup2(sdo,1) == -1)
			perror("There was an error with dup2");
		
		delete[] argv;	
	}
	
	return 0;
}

void simpleFork(char **argv, bool BG){
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
	if(! BG){
		//error check parent
		if(wait(&status)==-1){
			perror("Error in wait");
			exit(1);
		}
	}
}

void getPipes(char** argv){
	bool pipeFound = false;
	//save spots for the beginning arguments and ending arguments
	char **args = argv;
	char **nextA = argv;
	
	//go through argv and check if pipes are found
	//if they are found, it goes into function forkpipes
	for(int i=0; argv[i] != '\0'; i++){
		if(strcmp(argv[i], "|") == 0){
			pipeFound=true;
			argv[i] = '\0';
			char** beforeA = args;
			nextA = args + i + 1;
			forkPipes(beforeA, nextA);
			break;
		}
	}
	//if pipe isn't found, it goes into the simple fork
	if(!pipeFound){
		bool BG = false;
		for(int i=0; nextA[i] != '\0'; i++){
			if(strcmp(nextA[i], "&") == 0)
				BG=true;
		}
		simpleFork(nextA, BG);
	}
}

//execution if the command line has pipes
void forkPipes(char **argv, char **afterPipe){
	int fd[2];
	int pid;
	//pipe new file descriptors
	if(pipe(fd) == -1){
		perror("Error with pipe.");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if(pid == -1){
		perror("There was an error with piping fork.");
		exit(1);
	}
	//in child
	else if(pid ==0){
		//close stdin
		if(close(fd[0]) == -1){
			perror("Error with close in piping.");
			exit(1);
		}
		//save stdout
		if(dup2(fd[1],1)==-1){
			perror("Error with dup2 in piping.");
		}
		//execute
		if(execvp(argv[0], argv) == -1){
			perror("There was an error with execvp piping");
			exit(1);
		}
		else{
			exit(0);
		}	
	}
	
	//in parent
	//save stdin
	int save=dup(0);
	
	if((save== -1))
		perror("There was an error with dup.");
	//close std out
	if(close(fd[1]) == -1){
		perror("Error with close in piping.");
		exit(1);
	}
	//make stdin with dup2	
	if(dup2(fd[0],0) == -1){
		perror("Error with dup2 in piping.");
	}

	//make sure the parent waits
	if(wait(0) == -1)
		perror("There was an error in wait.");

	//while(waitpid(-1, &status, 0) >= 0);
	//check for pipes after we execute
	getPipes(afterPipe);
	
	if(dup2(save,0)==-1){
		perror("Error with dup2 in piping.");
	}
}

//deals with the IO redirection
void IOredir(char *argv[]){
	//check if there are any I/O redirection symbols
	for(int i=0; argv[i] != '\0'; i++){
		//check for "<" output redir
		if(strcmp(argv[i], "<") == 0){
			//open the file for read only
			int file = open(argv[i+1], O_RDONLY);
			if(file == -1)
				perror("There was an error with open");
			if(dup2(file, 0)==-1)
				perror("There was an error in dup2");
			argv[i] = NULL;
			break;
		}
		//check for ">" input redir
		else if(strcmp(argv[i], ">") == 0){
			//open the file, create or truncate and make write only
			int file = open(argv[i + 1], O_CREAT|O_TRUNC|O_WRONLY, 0666);
			if(file ==-1)
				perror("There was an error with open");
			if(dup2(file, 1)== -1)
				perror("There was an error with dup2");
			argv[i] = NULL;
			break;
		}
		//check for appending ">>" input redir
		else if(!strcmp(argv[i], ">>")){
			argv[i] = NULL;
			//open the file, create or append to old file and make write only
			int file = open(argv[i+1], O_CREAT|O_WRONLY|O_APPEND, 0666);
			if(file == -1)
				perror("There was an error with open");
			if(dup2(file,1) == -1)
				perror("There was an error with dup2");
			break;
		}
	}
}


void fixSpaces(string &input){
	for(unsigned int i = 0; i < input.size(); i++){
		//check if | is right next to word
		//if | is in the middle of a word
		if(input[i] == '|' && input[i+1] != ' ' && input[i-1] != ' ' && i !=0){
			input.insert(i+1, " ");
			input.insert(i, " ");
		}
		//if | is right before a word
		else if(input[i] == '|' && input[i+1] != ' ')
			input.insert(i+1, " ");
		//if | is right after word
		else if(input[i] == '|' && input[i-1] != ' ' && i!=0)
			input.insert(i, " ");
		
		//check if < is right next to word
		//if < is in the middle of a word
		if(input[i] == '<' && input[i+1] != ' ' && input[i-1] != ' ' && i !=0){
			input.insert(i+1, " ");
			input.insert(i, " ");
		}
		//if < is right before a word
		else if(input[i] == '<' && input[i+1] != ' ')
			input.insert(i+1, " ");
		//if < is right after word
		else if(input[i] == '<' && input[i-1] != ' ' && i!=0)
			input.insert(i, " ");
		
		
		//check if > is right next to word
		/*if( input[i-1] != '>' && input[i+1]!= '>'){
			//if > is in the middle of a word
			if(input[i] == '>' && input[i+1] != ' ' && input[i-1] != ' ' && i !=0){
				input.insert(i+1, " ");
				input.insert(i, " ");
			}
			//if > is right before a word
			else if(input[i] == '>' && input[i+1] != ' ')
				input.insert(i+1, " ");
			//if > is right after word
			else if(input[i] == '>' && input[i-1] != ' ' && i!=0)
				input.insert(i, " ");
		}
		
		//has to be >>
		if(input[i-1] != '>' && input[i+1] == '>'){
			//if >> is in the middle of a word
			if(input[i] == '>' && input[i+1] == '>' && input[i-1] != '>' && i !=0){
				input.insert(i-1, " ");
				input.insert(i, " ");
			}	
			//if | is right before a word
			else if(input[i] == '|' && input[i+1] != ' ')
				input.insert(i+1, " ");
			//if | is right after word
			else if(input[i] == '|' && input[i-1] != ' ' && i!=0)
				input.insert(i, " ");
			
		}*/
	}
}
