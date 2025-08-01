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
bool schedulerRunning = true;


void createProcess(Scheduler* scheduler, Config config) {
    static int totalCreated = 0;
    static int skipCount = 0;
    const int N = config.batch_process_freq;
	int max_frames = config.max_mem_per_proc / config.mem_per_frame;
	int min_frames = config.min_mem_per_proc / config.mem_per_frame;
    
	int randomPageCount = min_frames + (rand() % (max_frames - min_frames + 1));

    while (schedulerRunning) {
        if (skipCount < N) {
            ++skipCount;
            continue; // Skip this loop iteration to simulate delay
        }

        skipCount = 0; // Reset skip counter

        std::string processName = "SampleProcess" + std::to_string(totalCreated++);
        std::string processTime = getCurrentTime();
        int instructionCount = config.min_ins + (rand() % (config.max_ins - config.min_ins + 1));

        scheduler->addQueue(Process(processName, processTime, instructionCount, randomPageCount));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // PARA D SUMABOG PC NYO WAG NYO MASYADO BABAAN
    }
}

int mainMenu() {
    bool initialized = false;
    Scheduler* scheduler = nullptr;

    std::map<std::string, int> stringMap = {
        {"initialize",1},
        {"screen-ls",2},
        {"scheduler-start",3},
        {"scheduler-stop",4},
        {"report-util",5},
        {"clear",6},
        {"exit",7},
        {"vmstat",11},
        {"process-smi",12}
    };

    std::string command;
    int choice = 6;

    std::string processName;
    std::regex screenR(R"(screen -r (\w+))");
    std::regex screenS(R"(screen -s (\w+)(?:\s+(\d+))?)");
    std::smatch match;

    std::system("chcp 65001");
    clearScreen();
    std::srand(std::time(nullptr));
    Config config;

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
                config = initConfig("config.txt");
                initialized = true;
                // total frames
				int total_frames = config.max_overall_mem / config.mem_per_frame;

                scheduler = new Scheduler(config.num_cpu, config.scheduler, config.quantum_cycles,config.delay_per_exec,total_frames);
                scheduler->setConfig(config);

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

            case 3:{
                std::cout << "Generating sample processes.\n";
                std::thread schedulerStartThread(createProcess, scheduler, config);
                schedulerStartThread.detach();
                break;
            }
            case 4:{
                std::cout << "Scheduler-stop command recognized. Terminating Process Generation\n";
                schedulerRunning = false;
                break;
            }
            case 5:
                std::cout << "Report-util command recognized. Doing something.\n";
                scheduler->printSystemStatusToFile();
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

                int instructionCount = config.min_ins + (std::rand() % (config.max_ins - config.min_ins + 1));

                std::string processName = match[1];
                int requestedMemory;

                if (match.size() > 2 && match[2].matched) {
                    // Specific use case: screen -s name memory
                    requestedMemory = std::stoi(match[2]);

                    // Validate memory: must be a power of 2 and between 64 and 65536 bytes
                    if (requestedMemory < 64 || requestedMemory > 65536 || (requestedMemory & (requestedMemory - 1)) != 0) {
                        std::cout << "Invalid memory allocation: must be a power of 2 between 64 and 65536 bytes.\n";
                        choice = 6;
                        break;
                    }
                } else {
                    // General use case: screen -s name
                    int max_frames = config.max_mem_per_proc / config.mem_per_frame;
                    int min_frames = config.min_mem_per_proc / config.mem_per_frame;
                    int randomPageCount = min_frames + (rand() % (max_frames - min_frames + 1));
                    requestedMemory = randomPageCount * config.mem_per_frame;
                }

                int requiredPages = requestedMemory / config.mem_per_frame;
                if (requestedMemory % config.mem_per_frame != 0) {
                    requiredPages += 1;
                }

                std::cout << "Creating a new process...\n\n";
                std::cout << "Process Name: " << processName << "\n";
                std::cout << "Total line of instruction: " << instructionCount << "\n";
                std::cout << "Process Time: " << processTime << "\n";
                std::cout << "Requested Memory: " << requestedMemory << " bytes (" << requiredPages << " page(s))\n\n";
                
                scheduler->addQueue(Process(processName, processTime, instructionCount, requiredPages));
                scheduler->attachToScreen(processName);  // directly enter screen after creation

                choice = 6;
                break;
            }

            case 10:
                break;
            case 11:
                scheduler->printVMStat();
				break;
            case 12:
                scheduler->printProcessSMI();
				break;    
        }

    } while (choice != 7);

    return 0;
}



#endif
