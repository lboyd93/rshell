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

void permission(const struct stat buf, dirent *dirp);
void ls_a(const char* path){
	DIR *dirp;
	if(NULL == (dirp = opendir(path))){
		perror("There was an error with opendir().");
		exit(1);
	}
	struct dirent *filespecs;
	errno = 0;
	while( (filespecs = readdir(dirp) ) != NULL){
		cout << filespecs -> d_name << "  " ;
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

void ls_l(const char* dir){
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
		permission(s, filespecs);
		//cout << endl;
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

void permission(const struct stat buf, dirent *dirp){
	struct tm* file_t;
	file_t = localtime( &buf.st_mtime);
	char buffer[100];
	strftime(buffer, 100, "%h %e %R", file_t);
	
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
	vector<char*> dir_names;
	vector<char*> file_names;
	//char* path;
	
	//set flag as you check which kind of argument is input
	int flag=0;
	int numFile=1;
	//make first spot in dir_names '.'
//	dir_names.push_back(".");
	
	//check if the arguments are flags or filenames
	
	for(int pos=1; pos<argc; pos++){
		if(!strcmp(argv[pos],"-a")){
			flag = flag | 0x01;
			if(argc <=2)
				ls_a(".");
		//	ls_a(path);
		}
		else if(!strcmp(argv[pos], "-l")){ 
			flag = flag | 0x02;
			if(argc <= 2)
				ls_l(".");
		}
		else if(!strcmp(argv[pos], "-R")){
			flag = flag | 0x04;
		}
		else{
			if(numFile == 1){
				string dir_name = argv[pos];
				//dir_names[0] = dir_name;
				numFile++;
			}
			else{ dir_names.push_back(argv[pos]); }
		}
	}
	//cout << flag << endl;	
	
	return 0;
}
