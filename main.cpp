#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{	
	string input;
	cout << "$";	
	while(cin >> input ) {
		cout << "$";
		if(input == "exit")
			return 0;
	}
	
	return 0;
}
