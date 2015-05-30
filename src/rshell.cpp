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
#include <fcntl.h>      //flags
#include <sys/stat.h>   //stat
#include <signal.h>     //sigaction

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
void forkPipes(char **argv, char **afterPipe, char **path);
void simpleFork(char **argv, char **path);
void getPipes(char** argv, char **path);
void execPath(char **parsedpath, char **argv);
void cdCheck(char **argv, string input, char *currDir);

int current = 0;
unsigned char sig_flag = 0;
//interupting ^C signal, do nothing when ^C pushed
static void handleC(int sigNum){
    //cout << "Caught SIGINT, do nothing now." << endl;
    if(sigNum == SIGINT)
    {
        cout << endl;
        //kill(current, SIGKILL); 
        //signal(SIGINT, SIG_IGN);
    }
   // else if( sigNum == SIGTSTP){
   //     cout << endl;
   //     if(current != 0){
   //         kill(current, SIGSTOP);
     //   }
}

//parses the path so you know which commands you are working with
void fixPath(char *p, char **newPath){
    int numPath=0;
    newPath[numPath] = strtok(p,":");
    while(newPath[numPath] != NULL){
        numPath++;
        newPath[numPath] = strtok(NULL, ":");
    }
    return;
}

//changes 'HOME' to '~'
void findHome(string &dir, string home, string tilda){
    dir.replace(dir.find(home), home.length(), tilda);
}


int main(){
    //get the path we are in
    char *path = getenv("PATH");
    if(path == NULL)
        perror("There was an error with getenv");

    //parse the path
    char *newPath[BUFSIZ];
    fixPath(path, newPath);
    
    //make our sigaction struct
    // struct sigaction oldAction, newAction;

    //set the sig handler to SIG_IGN
    //newAction.sa_handler = SIG_IGN;

    //sigemptyset (&newAction.sa_mask);
    //newAction.sa_flags = 0;

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
        //get the current working directory
        char currDir[BUFSIZ];
        if(getcwd(currDir, sizeof(currDir)) == NULL)
            perror("Error with getcwd");
        string curr_dir = currDir;
        char *home = getenv("HOME");
        //find and change HOME to '~'
        if(home == NULL)
            perror("Error with getenv");
        findHome(curr_dir, home, "~");
        
        //cout << curr_dir << endl;

        cout << login << "@" << host << ":" << curr_dir << "$ ";

        //get the current directory as a c_str
        char *dir = new char[curr_dir.size() + 1];
        copy(curr_dir.begin(), curr_dir.end(), dir);
        dir[curr_dir.size()] = '\0'; 

        //helps fix infinite loop from signal
        cin.clear();   

        //make our sigaction struct
        struct sigaction oldaction, newAction;

        //set the sig handler to SIG_IGN
        newAction.sa_handler = handleC;

        //sigemptyset (&newAction.sa_mask);
        newAction.sa_flags = SA_SIGINFO;

        //sets handler to ignore function
        int sa=sigaction(SIGINT,&oldaction,NULL);
        if(sa==-1){
            perror("Error with sigaction");
            exit(1);
        }
        if ( sa  == 0 && oldaction.sa_handler != SIG_IGN){
        //newAction.sa_handler = handleC;
            if(sigaction(SIGINT, &newAction, NULL)==-1)
               perror("Error with sigaction");
        }
        /*
        if((sa = sigaction(SIGTSTP, &oldaction, NULL)))
                perror("sigaction");
       if(oldaction.sa_handler != SIG_IGN){
            if(-1 == (sa = sigaction(SIGTSTP, &newAction, NULL))){
                perror("sigaction");
            }
        */
        
        //signal(SIGINT, handleC);
        string input;
        char buf[1024];

        int sdi = dup(0), sdo = dup(1);
        if(sdi == -1)
            perror("There was an error with dup");
        if(sdo == -1)
            perror("There was an error with dup");	

        //get the input from user
        getline(cin, input);

        //check for comments
        checkcomm(input);

        //check if user pushes enter 
        if(input == "")
            continue;

        //checks the spacing
        fixSpaces(input);

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

        //check for cd
        if(!strcmp(argv[0], "cd")){
            cdCheck(argv, input, currDir);
            continue;
        }

        //for(unsigned i=0; argv[i] != NULL ; i++)
        //    cout << argv[i] << ' ';
        //cout << endl;

        //check IO redirection and pipes
        IOredir(argv);
        getPipes(argv, newPath);

        if(dup2(sdi, 0) == -1)
            perror("There was an error with dup2");
        if(dup2(sdo,1) == -1)
            perror("There was an error with dup2");

        delete[] dir;
        delete[] argv;	
    }

    return 0;
}

void simpleFork(char **argv, bool BG, char **path){
    int status = 0;
    int i = fork();
    if(i == -1){
        //error check fork
        perror("There was an error with fork()");
        exit(1);
    }
    else if(i == 0) {
        execPath(path, argv);
    }

    int wpid;
    //in parent
    if(! BG){
        do
        {
            wpid = wait(&status);
        }
        while (wpid == -1 && errno == EINTR);
        if (wpid == -1)
        {
            perror("wait error");
            exit(1);
        }
    }
}

void getPipes(char** argv, char **path){
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
            forkPipes(beforeA, nextA, path);
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
        simpleFork(nextA, BG, path);
    }
}

//execution if the command line has pipes
void forkPipes(char **argv, char **afterPipe, char **path){
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
        execPath(path, argv);
    }
    else
        exit(0);


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
    getPipes(afterPipe, path);

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

//does not work, too many syntax errors
/*void addPath(char newPath, char **p, char *command, int pos){

  strcpy(newPath, p[pos]);
  if(newPath[strlen(newPath)-1] != '/')
  strcat(newPath, "/");
  strcat(newPath, command);
  }*/

void execPath(char **p, char **argv){
    for(int pos=0; p[pos] != '\0'; pos++)
    {
        char path[BUFSIZ] = {0};
        char *args[BUFSIZ];

        // addPath(path, p, argv[0], pos);
        //copy the path onto the newpath
        strcpy(path,p[pos]);
        //append a '/' on the end of the path
        if(path[strlen(path)-1] != '/')
            strcat(path, "/");
        //append the command to the end
        strcat(path,argv[0]);

        //set our new command array to path
        args[0] = path;

        //go through the rest of the arguments and append it to new command array
        for(int index=1; argv[index] != NULL; index++)
            args[index] = argv[index];


        if(-1 == execv(args[0], args));
        // perror("Error with execv"); 
        else
            return;
    }

    //set and check errno
    if(errno > 0){
        perror("Error with execv");
        exit(1);
    }
}

//does a check for cd
void cdCheck(char **argv, string input, char *currDir){
    //this is when only 'cd' or 'cd ~'
    if(input.size() == 2 || (input.size() == 4 && !strcmp(argv[1], "~"))){
        //change to the home directory
        char *setDir = getenv("PWD");
        if(setDir == NULL){
            perror("There was an error with getenv");
        }
        //save OLDPWD
        if(setenv("OLDPWD", setDir, 1) == -1)
            perror("There was an error with setenv");
        setDir = getenv("HOME");
        if(setDir == NULL){
            perror("Error with getenv");
        }
        //set current to HOME
        if(setenv("PWD", setDir, 1) == -1)
            perror("Error with setenv");
        if(chdir(setDir) == -1)
            perror("Error with chdir");
    }
    //this is when 'cd -'
    else if(input.size() == 4 && !strcmp(argv[1],"-")){
        //spots to save old directory and new one
        char *setDir;
        char *setDir1;
        //save the current
        setDir = getenv("PWD");
        //save the old
        setDir1 = getenv("OLDPWD");
        if(setDir == NULL)
            perror("Error with getenv");
        if(setDir1 == NULL)
            perror("Error with getenv");
        //switch to old
        if(chdir(setDir1)== -1){
            perror("Error with chdir");
        }
        //swap the two saved
        if(setenv("PWD", setDir1, 1) == -1)
            perror("Error with setenv");
        if(setenv("OLDPWD", setDir, 1) == -1)
            perror("Error with setenv");
    }
    //this is for 'cd PATH'
    else if( strcmp(argv[1], "-") && strcmp(argv[1], "~") ){
        //save current
        char *setDir = getenv("PWD");
        if(setDir == NULL)
            perror("Error with getenv");
        //set old
        if(setenv("OLDPWD", setDir, 1) == -1)
            perror("Error with setenv");
        //set current
        if(setenv("PWD", argv[1], 1) == -1)
            perror("Error with setenv");
        //change to new
        if(-1 == chdir(argv[1]))
            perror("Error with chdir");
    }
    return;
}

