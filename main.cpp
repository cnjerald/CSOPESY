/*
How to compile

g++ main.cpp
or
g++ -std=c++20 main.cpp -o main

How to run
./a

*/

#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <regex>
#include <ctime>
#include <vector>

// Function declarations
int mainMenu();
void printHeader();
void clearScreen();
std::string getCurrentTime();

struct process {
	std::string name;
	int totalLines;
	std::string time;
};

int mainMenu() {
	// Storage for processes {TEMPORARY}
	std::vector<process> processes;


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

	std::string command;
	int choice = 6;


	// Regex for -r and -s.
	std::string processName;
	std::string processTime = getCurrentTime();
	std::regex screenR(R"(screen -r (\w+))"); // CG[0] All , CG[1] (\w+)
	std::regex screenS(R"(screen -s (\w+))");
	std::smatch match;

	clearScreen();

	do {
		// Dont remove.
		if (choice == 6) {
			printHeader();
		}

		std::cout << "Enter a command: ";
		std::getline(std::cin, command);

		// This is the error handling, if command not found in map, then -> set to -1.
		if (stringMap.count(command)) {
			choice = stringMap[command];
		}
		else if (std::regex_match(command,match, screenR)) {
			processName = match[1];
			choice = 8;
		}
		else if (std::regex_match(command,match, screenS)) {
			processName = match[1];
			choice = 9;
		}
		else {
			choice = -1;
		}

		switch (choice) {
		case -1:
			std::cout << "Unknown Command\n";
			break;
		case 1:
			std::cout << "Initialize command recognized. Doing something.\n";
			break;
		case 2:
			std::cout << "Screen command recognized. Doing something.\n";
			break;
		case 3:
			std::cout << "Scheduler-test command recognized. Doing something.\n";
			break;
		case 4:
			std::cout << "Scheduler-stop command recognized. Doing something.\n";
			break;
		case 5:
			std::cout << "Report-util command recognized. Doing something.\n";
			break;
		case 6:
			// Clear screen pathway.
			#ifdef _WIN32
    			std::system("cls");
			#else
    			std::system("clear");
			#endif
			break;
		case 7:
			// Exit pathway
			break;
		case 8:
			std::cout << "Retrieving a process...\n";
			for (int i = 0; i < processes.size();i++) {
				if (processes[i].name == processName) {
					std::cout << "Process Name: " << processes[i].name << "\n";
					std::cout << "Total lines of instruction: " << processes[i].totalLines << "\n";
					std::cout << "Process Time: " << processes[i].time << "\n";
					break;
				}
				else if (i == processes.size() - 1) {
					std::cout << "Process not found!\n";
				}
			}
			break;
		case 9:
			// This is the -s pathway, it creates a process.
			std::cout << "Creating a new process...\n";
			std::cout << "Process Name: " << processName << "\n";
			std::cout << "Total line of instruction: 100" << "\n";// Blatantly hard coded.
			std::cout << "Process Time: " << processTime << "\n";
			processes.push_back({ processName, 100, processTime }); // Hard coded total lines.
			break;
		default:
			std::cout << "Something bad happened!";
			return 0;
		}

	} while (choice != 7);

	return 0;

}

void clearScreen() {
	#ifdef _WIN32
		std::system("chcp 65001");
		std::system("cls");
	#else
		std::system("clear");
	#endif
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

// This function returns a string of the current time.
std::string getCurrentTime() {
	time_t timestamp;
	time(&timestamp);
	struct tm* localTime = localtime(&timestamp);

	char buffer[80];
	strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", localTime);

	return std::string(buffer);
}

int main() {
	mainMenu();

	return 0;
}