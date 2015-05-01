#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>

using namespace std;

#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4 

void permission(const struct stat buf, dirent *dirp);
string addC_str(const char *name, char d_name[]);

//function for ls and ls -a 
void ls(const char* path, bool isA){
	DIR *dirp;
	if(NULL == (dirp = opendir(path))){
		perror("There was an error with opendir().");
		exit(1);
	}
	struct dirent *filespecs;
	errno = 0;
	while( (filespecs = readdir(dirp) ) != NULL){
		if(isA){
			cout << filespecs->d_name << "  ";
		}
		else{
			 if (strcmp(filespecs->d_name, ".")!=0 
			&& strcmp(filespecs->d_name, "..")!=0 
			&& strcmp(filespecs->d_name, ".git") != 0){
			cout << filespecs -> d_name << "  " ;
			}
		}
	}
	if(errno != 0){
		perror("There was an error with readdir().");
		exit(1);
	}
	cout << endl;
	if(-1 == closedir(dirp)){
		perror("There was an error with closedir().");
		exit(1);
	}
	return;
}

//function for ls -l and also when ls -a -l is called
void ls_l(const char* dir, bool isA, bool isR){
	DIR *dirp;
	if(NULL == (dirp = opendir(dir))){
		perror("There was an error with opendir().");
		exit(1);
	}
	struct dirent *filespecs;
	struct stat s;
	//stat(dir, &s);
	errno = 0;
	while( (filespecs = readdir(dirp) ) != NULL){
		stat(dir, &s);
		if(isA){
			permission(s, filespecs);	
		}
		else if(isR){
			
			
			
	
		}
		else{
			if (strcmp(filespecs->d_name, ".")!=0 
				&& strcmp(filespecs->d_name, "..")!=0 
				&& strcmp(filespecs->d_name, ".git") != 0){
				permission(s, filespecs);
			}
		}	
	}
	if(errno != 0){
		perror("There was an error with readdir().");
		exit(1);
	}
	cout << endl;
	if(-1 == closedir(dirp)){
		perror("There was an error with closedir().");
		exit(1);
	}
	
	return;
}

//function for -R
void ls_R(const char* dir){
	DIR *dirp;
	if(NULL == (dirp = opendir(dir))){
		perror("There was an error with opendir().");
		exit(1);
	}
	struct dirent *filespecs;
	errno = 0;
	
	while( (filespecs=readdir(dirp)) != NULL){
		if(filespecs-> d_name[0] != '.'){
			string path;
			path += addC_str(dir, filespecs->d_name);
			
			cout << filespecs->d_name << ":" <<  endl; 
				
			if(filespecs-> d_type == DT_DIR)
				ls_R(path.c_str());
			
		}
		else{
			cout << filespecs->d_name << endl;
			
		}
	
	}
	if(errno != 0){
		perror("There was an error with readdir().");
		exit(1);
	}
	cout << endl;
	if(-1 == closedir(dirp)){
		perror("There was an error with closedir().");
		exit(1);
	}

	return;
}

//try to get print info function working so I can delete other functions
void printDir(const char *dir){
	DIR *dirp;
	if(NULL == (dirp = opendir(dir))){
		perror("There was an error with opendir().");
		exit(1);
	}
	struct dirent *filespecs;
	struct stat s;
	//stat(dir, &s);
	errno = 0;
	while( (filespecs = readdir(dirp) ) != NULL){
		if (strcmp(filespecs->d_name, ".")!=0 && strcmp(filespecs->d_name, "..")!=0 
			&& strcmp(filespecs->d_name, ".git") != 0){
		stat(dir, &s);
		permission(s, filespecs);
		}
	}
	if(errno != 0){
		perror("There was an error with readdir().");
		exit(1);
	}
	cout << endl;
	if(-1 == closedir(dirp)){
		perror("There was an error with closedir().");
		exit(1);
	}
	
	return;

}

//permissions to be outputted by -l
void permission(const struct stat buf, dirent *dirp){
	//create a struct with for the time
	struct tm* file_t;
	file_t = localtime( &buf.st_mtime);
	char buffer[100];
	strftime(buffer, 100, "%h %e %R", file_t);
	
	//output all stat
	(buf.st_mode & S_IFDIR)? cout << "d":
	(buf.st_mode & S_IFCHR) ? cout<< 'c':
	(buf.st_mode & S_IFBLK) ? cout<< 'b':
	(buf.st_mode & S_IFIFO) ? cout<< 'f':
	(buf.st_mode & S_IFSOCK) ? cout<<'s':
	(buf.st_mode & S_IFLNK) ? cout<< 'l':
	
	(buf.st_mode & S_IRUSR)? cout << "r": cout << "-";
	(buf.st_mode & S_IWUSR)? cout << "w": cout << "-";
	(buf.st_mode & S_IXUSR)? cout << "x": cout << "-";
	
	(buf.st_mode & S_IRGRP)? cout << "r": cout << "-";
	(buf.st_mode & S_IWGRP)? cout << "w": cout << "-";
	(buf.st_mode & S_IXGRP)? cout << "x": cout << "-";
	
	(buf.st_mode & S_IROTH)? cout << "r": cout << "-";
	(buf.st_mode & S_IWOTH)? cout << "w": cout << "-";
	(buf.st_mode & S_IXOTH)? cout << "x": cout << "-";
	cout << " ";
	
	cout << " " << buf.st_nlink << " ";
	cout << getpwuid(buf.st_uid)-> pw_name << " ";
	cout << getgrgid(buf.st_gid) -> gr_name << " ";
	cout << buf.st_size << " " << buffer << " " << dirp->d_name << endl;
}

int main(int argc, char* argv[]){
	//make a vector for the directory names and filenames inputted
	//into command line
	vector<char*> dir_names;
	vector<char*> file_names;
	//char* path;
	
	//set flags
	int flags=0;
	int numFile=1;
	//booleans to keep track of which flags are called	
	bool isA=false;
	bool isL=false;
	bool isR=false;
	
	//check if the arguments are flags or filenames
	for(int pos=1; pos<argc; pos++){
		if(!strcmp(argv[pos],"-a")){
			isA=true;
			flags= flags | FLAG_a;
			//ls_a(path);
		}
		else if(!strcmp(argv[pos], "-l")){ 
			isL=true;
			flags = flags | FLAG_l;
		}
		else if(!strcmp(argv[pos], "-R")){
			isR=true;
			flags = flags | FLAG_R;
		}
		else if(!strcmp(argv[pos],"-al") || !strcmp(argv[pos],"-la")){
			isA=true;
			isL=true;
			flags= flags | FLAG_a && FLAG_l;
			//ls_a(path);
		}
		else if(!strcmp(argv[pos],"-aR") || !strcmp(argv[pos],"-Ra")){
			isA=true;
			flags= flags | FLAG_a;
			//ls_a(path);
		}
		//if it is a file or directory
		else{
			if(numFile == 1){
				char *dir_name = argv[pos];
				dir_names[0] = dir_name;
				numFile++;
			}
			else{ dir_names.push_back(argv[pos]); }
		}
	}
	
	cout << "Outputting argv: ";
	for(unsigned i=0; argv[i] != '\0'; i++)
		cout <<  argv[i] << " ";
	cout << endl;
	
	for(unsigned int i=0; i < dir_names.size(); i++)
		cout << "Here is directory names: " <<  dir_names[0] << endl;
	//cout << flag << endl;	
	
	if(argc == 1)
		ls(".", isA);
	
	if(isA) ls(".", isA);
	
	if(isL)
		ls_l(".", isA, isR);
	if(isR)
		ls_R(".");
		
	return 0;
}

//function to add path names together
string addC_str(const char *name, char d_name[]){
	return string(name) + "/" + string(d_name);
}
