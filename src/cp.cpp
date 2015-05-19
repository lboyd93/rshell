#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "Timer.h"
#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

void one(char *in, char *out) {
	std::ifstream is;     // open file
	is.open(in, ifstream::in);
	std::ofstream outfile;
	outfile.open(out, ifstream::out);

	char c;
	while (is.get(c))          // loop getting single characters
		outfile.put(c);
	is.close();                // close file
	outfile.close();
}

void two(char *in, char *out) {
	char buf[BUFSIZ];
	int fd1 = open(in, O_RDONLY);
	if(fd1 == -1)
		perror("Open failed");
	int fd2 = open(out, O_RDWR | O_CREAT);
	if(fd2 == -1)
		perror("Open failed");

	int bytes_read, written;
	
	while((bytes_read = read(fd1, &buf, 1)) > 0) {
		if(bytes_read == -1){
			perror("There was an error with read");
		}
		written = write(fd2, &buf, 1);
		if(written != bytes_read)
			perror("Write failed");
	}
	
	
	if(close(fd1) < 0)
		perror("Close failed");
	if(close(fd2) < 0)
		perror("Close failed");
	
}

void three(char *in, char *out) {
		int fd1 = open(in, O_RDONLY);
		if(fd1 == -1)
			perror("Open failed");
		int fd2 = open(out, O_RDWR | O_CREAT);
		if(fd2 == -1)
			perror("Open failed");
	
		char buff[BUFSIZ];
		int n;
		while((n = read(fd1, &buff, BUFSIZ)) > 0){
			if(n == -1){
				perror("There was an error with read.");
			}
			if(write(fd2, &buff, n)==-1)
				perror("There was an error with write.");
		}
		
		if(close(fd1) < 0)
			perror("Close failed");
		if(close(fd2) < 0)
			perror("Close failed");
}	

int main(int argc, char *argv[]) {
	if(argc < 3) {
		cerr << "Error: not enough arguments" << endl;
		return 0;
	}
	if(argc > 4) {
		cerr << "Error: too many arguments" << endl;
		return 0;
	}
	struct stat buffer;
	Timer t;
	double wall1, user1, system1, wall2, user2, system2, wall3, user3, system3;
	
	char *in = argv[1];
	char *out = argv[2];
	int statin = stat(in, &buffer);
	int statout = stat(out, &buffer);
	
	if(statin == -1)
		perror("There was an error with stat.");
	if(statout == -1)
		perror("There was an error with stat.");
	
	
	if(statin != 0) {
		cerr << "Error: source file doesn't exist" << endl;
		return 0;
	}
	if(statout == 0) {
		cerr << "Error: destination file already exists" << endl;
		return 0;
	}
	if(argc == 4) {
		char *opt = argv[3];

		if(strcmp(opt, "fastest") == 0)
			three(in, out);
		else if(strcmp(opt, "run3") == 0) {
			t.start();
			one(in, out);
			t.elapsedTime(wall1, user1, system1);
			two(in, out);
			t.elapsedTime(wall2, user2, system2);
			three(in, out);
			t.elapsedTime(wall3, user3, system3);
	
			wall3 -= wall2;
			wall2 -= wall1;
			user3 -= user2;
			user2 -= user1;
			system3 -= system2;
			system2 -= system1;
		
			cout << "Method 1:\tWall- " << wall1 << endl;
			cout << "\t\tUser- " << user1 << endl;
			cout << "\t\tSystem- " << system1 << endl;
			cout << "Method 2:\tWall- " << wall2 << endl;
			cout << "\t\tUser- " << user2 << endl;
			cout << "\t\tSystem- " << system2 << endl;
			cout << "Method 3:\tWall- " << wall3 << endl;
			cout << "\t\tUser- " << user3 << endl;
			cout << "\t\tSystem- " << system3 << endl;
		}
	}
	else
		three(in, out);
	return 0;
}
