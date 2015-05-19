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

void outputcolors(dirent *direntp, struct stat s);
bool compareNoCase( const string& s1, const string& s2 );
void sortDir(const char* dir, vector<string> &dirlist, bool isA);
void permission(int &total,const struct stat buf);
string addC_str(const char *name, char d_name[]);
void getTotal(int &total, struct stat buf, dirent *dirp);
void printInfo(int &total, struct stat buf, dirent *dirp);

//function for ls and ls -a 
void ls(const char* path, bool isA, int size){
	DIR *dirp;
	if(NULL == (dirp = opendir(path))){
		perror("There was an error with opendir().");
		exit(1);
	}
	vector<string> dirList;
	
	sortDir(path, dirList, isA);
	
	struct dirent *filespecs;
	struct stat s;
	
	errno = 0;
	while( (filespecs = readdir(dirp) ) != NULL){
		if(stat(path, &s)== -1)
			perror("Error with stat");
		if(isA){
			cout << filespecs-> d_name << "  ";
		}
		else if(isA && size > 1){
			//cout << path << ":" << endl;
			cout << filespecs-> d_name << "  ";
		}
		else{
			 if (filespecs->d_name[0] != '.'){
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
	
	vector<string> dirlist;
	
	sortDir(dir, dirlist,isA);
	
	errno = 0;
	while(  (filespecs1=readdir(dirp)) != NULL){
		if(stat(dir, &s)==-1)
			perror("Error with stat");
		getTotal(total, s, filespecs1);
	}
	
	if(errno != 0){
		perror("There was an error with readdir().");
		exit(1);
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
		//if(filespecs2 == -1)
		//	perror("There was an error with readir");
		if(stat(dir, &s)==-1)
			perror("There was an error with stat");
		if(isA){
			//cout << "Total: " << total/2 << endl;
			printInfo(total,s,filespecs2);
		}
		else if(isR){
			
		//TO DO	
			
	
		}
		if(!isA && !isR){
			if (filespecs2-> d_name[0] != '.'){
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


bool compareNoCase( const string& s1, const string& s2 ) {
    return strcasecmp( s1.c_str(), s2.c_str() ) <= 0;
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
		if(filespecs1->d_name[0] == '.' && !isA) continue;
		dirlist.push_back(filespecs1->d_name);
		i++;
	}
	
	sort(dirlist.begin(), dirlist.end(), compareNoCase);
		
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
	if(stat(dirp->d_name, &buf)==-1)
		perror("Error with stat");
	
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
	cout << buf.st_size << " " << buffer << " "; 
	outputcolors(dirp, buf)  ;
	cout << endl;
}

void getTotal(int &total, struct stat buf, dirent *dirp){
	total += buf.st_blocks;
	return;

}

//permissions to be outputted by -l
void permission(int &total,struct stat buf){
	
	//output all stat
	(buf.st_mode & S_IFREG) ? cout<< '-':
	(buf.st_mode & S_IFDIR)? cout << 'd':
	(buf.st_mode & S_IFCHR) ? cout<< 'c':
	(buf.st_mode & S_IFBLK) ? cout<< 'b':
	cout << '-';	
	
	(buf.st_mode & S_IRUSR)? cout << 'r': cout << '-';
	(buf.st_mode & S_IWUSR)? cout << 'w': cout << '-';
	(buf.st_mode & S_IXUSR)? cout << 'x': cout << '-';
	
	(buf.st_mode & S_IRGRP)? cout << 'r': cout << '-';
	(buf.st_mode & S_IWGRP)? cout << 'w': cout << '-';
	(buf.st_mode & S_IXGRP)? cout << 'x': cout << '-';
	
	(buf.st_mode & S_IROTH)? cout << 'r': cout << '-';
	(buf.st_mode & S_IWOTH)? cout << 'w': cout << '-';
	(buf.st_mode & S_IXOTH)? cout << 'x': cout << '-';
	cout << ' ';
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
			isR=true;
		}
		else if(!strcmp(argv[pos],"-aRl") || !strcmp(argv[pos],"-Ral") 
			|| !strcmp(argv[pos],"-alR") || !strcmp(argv[pos],"-lRa")
				|| !strcmp(argv[pos],"-laR") || !strcmp(argv[pos],"-Rla")){
			isA=true;
			isR=true;
			isL=true;
		}
		//if it is a file or directory
		else{
			dir_names.push_back(argv[pos]); 
		}
	}
	
	//sorting directories
	/*sort(dir_names.begin(), dir_names.end(), compareNoCase);
	
	cout << "Outputting sorted Directories: ";
	for(unsigned int i=0; i< dir_names.size(); i++)
	{
		cout << dir_names[i] << " ";
	}
	cout << endl;*/
	int size = dir_names.size();	
	if(size== 0){
		if( !isA && !isL && !isR)
			ls(".", 0, size);
		
		else if(isA && !isR && !isL) ls(".", 1, size);
		
		else if(isA && isL && !isR) ls_l(".", 1,0);
	
		else if(isL && !isA && !isR)
			ls_l(".", 0, 0);
		else if(isR && !isA && !isL)
			ls_R(".", 0, 0);
		else if(isR && isA && !isL)
			ls_R(".", 1, 0);
		else if(isR && isA && isL)
			ls_R(".", 1, 1);
	}
		
	for(unsigned int i =0; i < dir_names.size(); i++){
		
		if( !isA && !isL && !isR)
			ls(dir_names[i], isA, size);
		
		else if(isA && !isR&& !isL) ls(dir_names[i], isA, size);
		
		else if(isA && isL && !isR) ls_l(dir_names[i], 1,0);		

		else if(isL && !isA && !isR)
			ls_l(dir_names[i], 0, 0);
		else if(isR && !isA && !isL)
			ls_R(dir_names[i], 0, 0);
		else if(isR && isA && !isL)
			ls_R(dir_names[i], 1, 0);
		else if(isR && isA && isL)
			ls_R(dir_names[i], 1, 1);
	}
	return 0;
}

//function to add path names together
string addC_str(const char *name, char d_name[]){
	return string(name) + "/" + string(d_name);
}

void outputcolors(dirent *direntp, struct stat s)
{
	//grey back, blue words
	if(direntp->d_name[0] == '.' && (s.st_mode & S_IFDIR)){
		cout<<"\033[47m\033[38;5;32m"<<direntp->d_name<<"\033[0;00m";
	}
	//grey back ground green words
	else if(direntp->d_name[0] == '.' && (s.st_mode & S_IXUSR)){
		cout<<"\033[47m\033[38;5;34m"<<direntp->d_name<<"\033[0;00m";
	}

	else if(direntp->d_name[0] == '.'){
		cout<<"\033[47m"<<direntp->d_name<<"\033[0;00m";
	}
	//directory
	else if(s.st_mode & S_IFDIR){
		cout<<"\033[38;5;32m"<<direntp->d_name<<"\033[0;00m";
	}
	//executable
	else if(s.st_mode & S_IXUSR){
		cout<<"\033[38;5;34m"<<direntp->d_name<<"\033[0;00m";
	}
	else
	{
		cout<<direntp->d_name;	
	}
	if(s.st_mode & S_IFDIR) cout<< '/';
	else if(s.st_mode & S_IXUSR) cout<< '*';
	cout<<"  ";
}
