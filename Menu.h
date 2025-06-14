#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <regex>
#include <ctime>
#include <vector>
#include "Process.h"
#include "Utils.h"
#include "InitConfig.h"
#include "scheduler.h"
int mainMenu();

int mainMenu() {
	//This is for initialization.
	bool initialized = false;

	// Storage for processes {TEMPORARY}
	std::vector<Process> processes;

	Scheduler* scheduler = nullptr;

	std::map<std::string, int> stringMap = {
		{"initialize",1},
		{"screen -ls",2},
		{"scheduler-start",3},
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

	std::system("chcp 65001");
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

		if (!initialized && choice > 1 && choice < 6) {
			std::cout << "You must initialize the system first by typing 'initialize'.\n";
			choice = 10; // Reset choice to main menu
		}

		switch (choice) {
			case -1:
				std::cout << "Unknown Command\n";
				break;
			case 1: {
				Config config = initConfig("config.txt");
				initialized = true;
				scheduler = new Scheduler(config.num_cpu, config.scheduler, config.quantum_cycles);

				std::thread cycleThread(&Scheduler::runOneCycleLoop, scheduler);
				cycleThread.detach();

				std::thread queueThread(&Scheduler::checkQueueLoop, scheduler);
				queueThread.detach();

				std::cout << "Initialized Successful.\n";
				break;
			}
			case 2:
				scheduler->printProcessQueue();
				scheduler->printCurrentProcess();
				scheduler->printFinishedProcesses();
				break;
			case 3:
				std::cout << "Creating 10 sample processes.\n";
				for (int i = 0; i < 10; ++i) {
					std::string processName = "SampleProcess" + std::to_string(i);
					std::string processTime = getCurrentTime();
					scheduler->addQueue(Process(processName, processTime, 20));
				}
				break;
			case 4:
				std::cout << "Scheduler-stop command recognized. Doing something.\n";
				break;
			case 5:
				std::cout << "Report-util command recognized. Doing something.\n";
				break;
			case 6:
				clearScreen();
				break;
			case 7:
				delete scheduler;
				scheduler = nullptr;
				break;
			case 8: {


				bool found = false;

				for (const auto& proc : processes) {
					if (proc.getName() == processName) {
						clearScreen();
						std::cout << "Retrieving a process...\n";
						std::cout << "Process Name: " << proc.getName() << "\n";
						std::cout << "Total lines of instruction: " << proc.getTotalLines() << "\n";
						std::cout << "Process Time: " << proc.getTime() << "\n\n";
						found = true;
						std::string input;
						do {
							std::cout << "Type 'exit' to go back to main menu: ";
							std::getline(std::cin, input);

							if (input != "exit") {
								std::cout << "Unknown command\n";
							}
						} while (input != "exit");

						clearScreen();
						
						choice = 6;
						break;
					}
				}
				if (!found) {
					std::cout << "Process \"" << processName << "\" not found.\n";
					std::cout << "The user can access the screen anytime by typing \"screen -r <process name>\" in the main menu.\n";
				}
				break;
			}
			case 9: {
				clearScreen();

				// This is the -s pathway, it creates a process.
				std::cout << "Creating a new process...\n\n";
				std::cout << "Process Name: " << processName << "\n";
				std::cout << "Total line of instruction: 10" << "\n"; // Hard coded
				std::cout << "Process Time: " << processTime << "\n\n";

				scheduler->addQueue(Process(processName, processTime, 10)); // Add process to scheduler queue

				std::string input;
				do {
					std::cout << "Type 'exit' to go back to main menu: ";
					std::getline(std::cin, input);

					if (input != "exit") {
						std::cout << "Unknown command\n";
					}
				} while (input != "exit");

				clearScreen();

				choice = 6;
				break;
			}
			case 10: {
				break;
			}
		}

	} while (choice != 7);

	return 0;

}

#endif