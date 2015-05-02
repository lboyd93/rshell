#include <iostream>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <algorithm>
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

void permission(int &total,const struct stat buf);
string addC_str(const char *name, char d_name[]);
void getTotal(int &total, struct stat buf, dirent *dirp);
void printInfo(int &total, struct stat buf, dirent *dirp);

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
	struct dirent *filespecs1;
	struct stat s;
//	stat(dir, &s);
	errno = 0;
	int total=0;
	while(  (filespecs1=readdir(dirp)) != NULL){
		stat(dir, &s);
		getTotal(total, s, filespecs1);
	}
	
	if(-1 == closedir(dirp)){
		perror("There was an error with closedir().");
		exit(1);
	}
	
	cout << "Total: " << total/2 << endl;
	
	if(NULL == (dirp = opendir(dir))){
		perror("There was an error with opendir().");
		exit(1);
	}

	struct dirent *filespecs2;
		
	while( (filespecs2 = readdir(dirp) ) != NULL){
		stat(dir, &s);
		if(isA){
			//cout << "Total: " << total/2 << endl;
			printInfo(total,s,filespecs2);
		}
		else if(isR){
			
		//TO DO	
			
	
		}
		else{
			if (strcmp(filespecs2->d_name, ".")!=0 
				&& strcmp(filespecs2->d_name, "..")!=0 
				&& strcmp(filespecs2->d_name, ".git") != 0){
				//cout << "Total: " << total/2 << endl;
				printInfo(total,s, filespecs2);
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

void sortDir(const char* dir, vector<string> &dirlist, bool isA){
	DIR *dirp;
	if(NULL == (dirp = opendir(dir))){
		perror("There was an error with opendir().");
		exit(1);
	}
	struct dirent *filespecs1;
	
	errno = 0;
	int i = 0;
	while( (filespecs1=readdir(dirp)) != NULL){
		if(!isA){
			if(!strcmp(filespecs1->d_name, ".") )
				continue;
		}
		dirlist.push_back(filespecs1->d_name);
		i++;
	}
	
	sort(dirlist.begin(), dirlist.end());
		
	if(errno != 0){
		perror("There was an error with readdir().");
		exit(1);
	}
	
	if(-1 == closedir(dirp)){
		perror("There was an error with closedir().");
		exit(1);
	}
	
	return;
}

//function for -R
void ls_R(const char* dir, bool isA, bool isL){
	vector<string> dirlist;
	DIR *dirp;
	if(NULL == (dirp = opendir(dir))){
		perror("There was an error with opendir().");
		exit(1);
	}
	struct dirent *filespecs;
	
	errno = 0;
	
	sortDir(dir, dirlist, isA);
	
	cout << dir << ":" << endl;
	
	for(unsigned int i=0; i < dirlist.size(); i++)
		cout << dirlist[i] << " ";
	cout << endl << endl;

	while( (filespecs=readdir(dirp)) != NULL){
		if(filespecs-> d_name[0] != '.'){
			string path;
			path += addC_str(dir, filespecs->d_name);
				
			if(filespecs-> d_type == DT_DIR)
				ls_R(path.c_str(), 1, 0);
		}
	}
/*	if (strcmp(filespecs->d_name, ".")!=0 
			&& strcmp(filespecs->d_name, "..")!=0 
			&& strcmp(filespecs->d_name, ".git") != 0){	
		filespecs=filespecs->filespecs;
	}	
	cout << dir << ":" << endl;
		
	for(unsigned int i=0; i < dirlist.size(); i++)
		cout << dirlist[i] << " ";
	cout << endl << endl;
	
	while( (filespecs=readdir(dirp)) != NULL){
		if(filespecs-> d_name[0] != '.'){
			string path;
			path += addC_str(dir, filespecs->d_name);
			if(filespecs-> d_type == DT_DIR)
				ls_R(path.c_str(), 0, 0);
		}
	}*/
	
	
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

void printInfo(int &total, struct stat buf, dirent *dirp){
	stat(dirp->d_name, &buf);
	
	//create a struct with for the time
	struct tm* timeinfo;
	timeinfo = localtime( &buf.st_mtime);
	char buffer[20];
	strftime(buffer, 20, "%b %d %H:%M", timeinfo);
	
	struct passwd *pw;
	if(!(pw = getpwuid(buf.st_uid)))
		perror("there was an error with getpwuid");
	struct group *gp;
	if(!(gp= getgrgid(buf.st_gid)))
		perror("there was an error with getgrgid");
	
	permission(total, buf);
	
	cout << " " << buf.st_nlink << " ";
	cout << pw-> pw_name << " ";
	cout << gp -> gr_name << " ";
	cout << buf.st_size << " " << buffer << " " << dirp->d_name << endl;
}

void getTotal(int &total, struct stat buf, dirent *dirp){
	total += buf.st_blocks;
	return;

}

//permissions to be outputted by -l
void permission(int &total,struct stat buf){
	
	//output all stat
	(buf.st_mode & S_IFDIR)? cout << "d":
	(buf.st_mode & S_IFCHR) ? cout<< 'c':
	(buf.st_mode & S_IFBLK) ? cout<< 'b':
	(buf.st_mode & S_IFIFO) ? cout<< 'f':
	(buf.st_mode & S_IFREG) ? cout<< '-':
	
	
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
}

int main(int argc, char* argv[]){
	//make a vector for the directory names and filenames inputted
	//into command line
	vector<char*> dir_names;
	vector<char*> file_names;
	vector<char*> others;
	//char* path;
	
	//int numArgs=1;
	
	//set flags
	int flags=0;
	//int numFile=1;
	//booleans to keep track of which flags are called	
	bool isA=false;
	bool isL=false;
	bool isR=false;
	
	//check if the arguments are flags or filenames
	for(int pos=1; pos<argc; pos++){
		if(!strcmp(argv[pos],"-a")){
			isA=true;
			//flags= flags | FLAG_a;
			//ls_a(path);
		}
		else if(!strcmp(argv[pos], "-l")){ 
			isL=true;
			//flags = flags | FLAG_l;
		}
		else if(!strcmp(argv[pos], "-R")){
			isR=true;
			//flags = flags | FLAG_R;
		}
		else if(!strcmp(argv[pos],"-al") || !strcmp(argv[pos],"-la")){
			isA=true;
			isL=true;
			//flags= flags | FLAG_a && FLAG_l;
			//ls_a(path);
		}
		else if(!strcmp(argv[pos],"-aR") || !strcmp(argv[pos],"-Ra")){
			isA=true;
			flags= flags | FLAG_a;
			//ls_a(path);
		}
		//if it is a file or directory
		else{
			dir_names.push_back(argv[pos]); 
		}
	}
	
	//sorting directories
	for(unsigned int i=0; i< dir_names.size(); i++)
		sort(dir_names.begin(), dir_names.end());
	
	
	if(argc == 1)
		ls(".", isA);
	for(unsigned int i =0; i < dir_names.size(); i++){
		
		if( !isA && !isL && !isR)
			ls(dir_names[i], isA);
		
		if(isA && !isR) ls(dir_names[i], isA);
	
		if(isL)
			ls_l(dir_names[i], isA, isR);
		if(isR && !isA && !isL)
			ls_R(dir_names[i], 0, 0);
		if(isR && isA && !isL)
			ls_R(dir_names[i], 1, 0);
		if(isR && isA && isL)
			ls_R(dir_names[i], 1, 1);
	}
	return 0;
}

//function to add path names together
string addC_str(const char *name, char d_name[]){
	return string(name) + "/" + string(d_name);
}
