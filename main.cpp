/*
How to compile

g++ main.cpp
or
g++ -std=c++11 main.cpp -o main

How to run
./a

*/

#include <iostream>
#include <string>
#include <map>
#include <cstdlib>

// Function declarations
int mainMenu();
void printHeader();

int mainMenu() {

	std::string command;

	// Put additional inputs here...
	std::map<std::string, int> stringMap = {
		{"initialize",1},
		{"screen",2},
		{"scheduler-test",3},
		{"scheduler-stop",4},
		{"report-util",5},
		{"clear",6},
		{"exit",7}
	};

	int choice;
	std::system("chcp 65001");
	std::system("cls");

	do {
		printHeader();
		std::cout << "Enter a command: ";
		std::cin >> command;

		// This is the error handling, if command not found in map, then -> set to -1.
		if (stringMap.count(command)) {
			choice = stringMap[command];
		}
		else {
			choice = -1;
		}

		switch (choice) {
		case -1:
			std::cout << "Unknown Command\n";
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			std::system("cls");
			break;
		case 7:
			break;
		default:
			std::cout << "Something bad happened!";
			return 0;
		}

	} while (choice != 7);

	return 0;

}

void printHeader() {
	std::cout << "░█████╗░░██████╗░█████╗░██████╗░███████╗░██████╗██╗░░░██╗\n";
	std::cout << "██╔══██╗██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝╚██╗░██╔╝\n";
	std::cout << "██║░░╚═╝╚█████╗░██║░░██║██████╔╝█████╗░░╚█████╗░░╚████╔╝░\n";
	std::cout << "██║░░██╗░╚═══██╗██║░░██║██╔═══╝░██╔══╝░░░╚═══██╗░░╚██╔╝░░\n";
	std::cout << "╚█████╔╝██████╔╝╚█████╔╝██║░░░░░███████╗██████╔╝░░░██║░░░\n";
	std::cout << "\033[32mHello, Welcome to CSOPESY commandline!\033[0m\n";
	std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen \033[0m\n";
}

int main() {

	mainMenu();

	return 0;
}