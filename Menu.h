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
    bool initialized = false;
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

    std::string processName;
    std::regex screenR(R"(screen -r (\w+))");
    std::regex screenS(R"(screen -s (\w+))");
    std::smatch match;

    std::system("chcp 65001");
    clearScreen();

    do {
        if (choice == 6) {
            printHeader();
        }

        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        if (stringMap.count(command)) {
            choice = stringMap[command];
        }
        else if (std::regex_match(command, match, screenR)) {
            processName = match[1];
            choice = 8;
        }
        else if (std::regex_match(command, match, screenS)) {
            processName = match[1];
            choice = 9;
        }
        else {
            choice = -1;
        }

        if (!initialized && choice > 1 && choice < 6) {
            std::cout << "You must initialize the system first by typing 'initialize'.\n";
            choice = 10;
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

            case 8: { // screen -r
                if (!scheduler) {
                    std::cout << "Scheduler is not initialized.\n";
                    break;
                }

                scheduler->attachToScreen(processName);
                choice = 6;
                break;
            }

            case 9: { // screen -s
                if (!scheduler) {
                    std::cout << "Scheduler is not initialized.\n";
                    break;
                }

                clearScreen();

                std::cout << "Creating a new process...\n\n";
                std::string processTime = getCurrentTime();
                std::cout << "Process Name: " << processName << "\n";
                std::cout << "Total line of instruction: 10\n";
                std::cout << "Process Time: " << processTime << "\n\n";

                scheduler->addQueue(Process(processName, processTime, 10));
                scheduler->attachToScreen(processName);

                choice = 6;
                break;
            }

            case 10:
                break;
        }

    } while (choice != 7);

    return 0;
}

#endif
