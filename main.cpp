/*
How to compile

g++ main.cpp

How to run
./a

*/ 

#include <iostream>
#include <string>
#include <map>

int mainMenu(){

	std::string command;

	// Put additional inputs here...
	std::map<std::string, int> stringMap ={
		{"clear",1},
		{"exits",2},
		{"banana",3}, // Change this to the other command
		{"potato",4} // Change this to the other command
	};

	int choice = 1;

	do{
		if(choice == 1){
			std::cout << "CSOPESY \n"; // Change this to the 2D version...
			std::cout << "Hello, Welcome to CSOPESY commandline! \n";
			std::cout << "Type 'exit' to quit, 'clear' to clear the screen \n";
		}

		std::cout << "Enter a command: ";
		std::cin >> command;

		// This is the error handling, if command not found in map, then -> set to -1.
		if(stringMap.count(command)){
			choice = stringMap[command];
		} else{
			choice = -1;
		}

		switch(choice){
			case -1:
				std::cout << "Unknown Command\n";
				break;
			case 1:
				std::system("cls");
				break;
			case 2:
				return 0;
				break;
			case 3:
			case 4:
			case 5:
			case 6:

		}

	} while (choice != 2);
	
	return 0;
	
}





int main(){
	
	mainMenu();

	return 0;
}