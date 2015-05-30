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
#include <signal.h>

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
void execArgs(char **parsedpath, char **argv);
void execPath(char *p, char **argv);

//interupting ^C signal, do nothing when ^C pushed
static void handleC(int sigNum){
    //cout << "Caught SIGINT, do nothing now." << endl;
    //exit(0);
    cout << endl;
}

//needed to keep track of paths
string nextDir;
string prevDir;

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

//does a check for cd
void cdCheck(char **argv, string input, char *currDir){
    //this is when only 'cd'
    if(input.size() == 2){
        //change to the home directory
        char *home = getenv("HOME");
        if(home == NULL){
            perror("There was an error with getenv");
        }
        if(chdir(home) == -1){
            perror("Error with chdir");
        }
        //change the directories
        prevDir = currDir;
        nextDir = home;
    }
    //this is when 'cd -'
    else if(input.size() == 4 && !strcmp(argv[1],"-")){
        //cout << "Here is the prevDir: " << prevDir << endl;
        //cout << "Here is nextDir: " << nextDir << endl;
        //change back to the previous directory
        if(chdir(prevDir.c_str())== -1){
            perror("Error with chdir");
        }
        //switch the nextdir with prevdir since we switched
        string curr = prevDir;
        prevDir = nextDir;
        nextDir = curr;

        //cout << "Here is the prevDir: " << prevDir << endl;
        //cout << "Here is newDir: " << nextDir << endl;
    }
    //this is for 'cd PATH'
    else if( strcmp(argv[1], "-")){
        if(-1 == chdir(argv[1])){
            perror("Error with chdir");
        }
        //keep track to previous and current directories
        prevDir = currDir;
        nextDir = argv[1];
    }
    return;
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
    struct sigaction oldAction, newAction;

    //set the sig handler to SIG_IGN
    newAction.sa_handler = SIG_IGN;

    sigemptyset (&newAction.sa_mask);
    newAction.sa_flags = 0;

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
        
        cout << login << "@" << host << ":" << curr_dir << "$ ";
        
        //get the current directory as a c_str
        char *dir = new char[curr_dir.size() + 1];
        copy(curr_dir.begin(), curr_dir.end(), dir);
        dir[curr_dir.size()] = '\0'; 

        //helps fix infinite loop from signal
        cin.clear();   

        //sets handler to ignore function
        int sa=sigaction(SIGINT,&newAction,&oldAction);
        if(sa==-1)
            perror("Error with sigaction");
        if ( sa  == 0 && oldAction.sa_handler != SIG_IGN){
            newAction.sa_handler = handleC;
            if(sigaction(SIGINT, &newAction, 0)==-1)
                perror("Error with sigaction");
        }

        string input;
        char buf[1024];

        int sdi = dup(0), sdo = dup(1);
        if(sdi == -1)
            perror("There was an error with dup");
        if(sdo == -1)
            perror("There was an error with dup");	

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
        execArgs(path, argv);
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
        execArgs(path, argv);
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

void addPath(char *newPath, char *path, char *command){
    
    strcpy(newPath, path);
    strcat(newPath, "/");
    strcat(newPath, command);
}

void execArgs(char **parsedpath, char **argv){
    for(int i=0; parsedpath[i] != '\0'; i++)
    {
        char check[250] = {0};

        strcpy(check,parsedpath[i]);
        if(check[strlen(check)-1] != '/')
            strcat(check, "/");
        strcat(check,argv[0]);


        char *newargv[50] = {0};
        newargv[0] = check;
        for(int j=1; argv[j] != NULL; j++)
            newargv[j] = argv[j];

        if(-1 == execv(newargv[0], newargv)) ; 
        else
            return;
    }
    if(errno)
    {
        perror("problem with execv. " );
        exit(1);
    }
}

//execs path
void execPath(char *p, char **argv){
    bool found = false;

    char *path = p;
    char *afterPath;

    for(int i = 0; path[i] != '\0'; ++i){
        if(path[i] == ':'){
            found = true;
            path[i] = '\0';

            char *beforePath = path;
            afterPath = i + path + 1;
            char newPath[BUFSIZ];
            addPath(newPath, beforePath, argv[0]);
            if(execv(newPath, argv) == -1){
                perror("Error with execv");
            }
            else return;

            execPath(afterPath, argv);
        }
    }

    if(!found){
        char newPath[BUFSIZ];
        addPath(newPath, path, argv[0]);
        if(execv(newPath, argv) == -1){
            perror("error with execv");
        }
    }
}
