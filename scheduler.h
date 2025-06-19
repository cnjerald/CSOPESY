#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <vector>
#include <string>
#include "Process.h"
#include "CPU.h"  // Include your CPU class
#include <thread>
#include <chrono>  // For sleep functionality

class Scheduler {
public:
    int num_cpu;
    int quantum_cycles;
    std::string scheduler;
    std::vector<Process> processQueue;
    std::vector<Process> finishedProcesses;  // Store finished processes
    std::vector<CPU> cpus;  // Each CPU holds a Process

    // Constructor
    Scheduler(int num_cpu, const std::string& scheduler, int quantum_cycles)
        : num_cpu(num_cpu), scheduler(scheduler), quantum_cycles(quantum_cycles)
    {
        for (int i = 0; i < num_cpu; i++) {
            cpus.emplace_back(i, quantum_cycles);
        }
    }

    // Check queue and assign available CPUs
    void checkQueue() {
        for (auto& cpu : cpus) {
            if (scheduler == "FCFS" && cpu.isAvailable() && !processQueue.empty()) {
                cpu.assignProcess(processQueue.front());
                processQueue.erase(processQueue.begin());
            }
        }
    }

    void printProcessQueue() {
        if (processQueue.empty()) {
            std::cout << "No processes in the queue.\n";
            return;
        }

        std::cout << "=== Current Process Queue ===\n";
        for (const Process& p : processQueue) {
            std::cout << "  Name: " << p.getName()
                      << " | Time: " << p.getTime()
                      << " | Progress: " << p.currentLine << "/" << p.getTotalLines()
                      << " | Status: " << p.status << "\n";
        }
    }

    void printCurrentProcess() {
        for (const auto& cpu : cpus) {
            if (!cpu.isAvailable()) {
                std::cout << "Current process on " << cpu.cpu_name << ":\n";
                const Process& p = cpu.getCurrentProcess();
                std::cout << "  Name: " << p.getName()
                          << " | Time: " << p.getTime()
                          << " | Progress: " << p.currentLine << "/" << p.getTotalLines()
                          << " | Status: " << p.status << "\n";
            }
        }
    }

    void printFinishedProcesses() {
        if (finishedProcesses.empty()) {
            std::cout << "No finished processes.\n";
            return;
        }

        std::cout << "=== Finished Processes ===\n";
        for (const auto& p : finishedProcesses) {
            std::cout << "  Name: " << p.getName()
                      << " | Time: " << p.getTime()
                      << " | Progress: " << p.currentLine << "/" << p.getTotalLines()
                      << " | Status: " << p.status << "\n";
        }
    }

    void addQueue(const Process& process) {
        processQueue.push_back(process);
        checkQueue();
    }

    // Simulate 1 clock cycle across all CPUs
    void runOneCycle() {
        for (auto& cpu : cpus) {
            cpu.oneClockCycle();

            // Check if this CPU has finished its assigned process
            if (cpu.isFinished()) {
                Process finished = cpu.retrieveFinishedProcess();
                finishedProcesses.push_back(finished);
            }
        }
    }

    void runOneCycleLoop() {
        while (true) {
            runOneCycle();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void checkQueueLoop() {
        while (true) {
            checkQueue();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    // ✅ Attach to screen with live reference
    void attachToScreen(const std::string& name) {
        // 1. Running
        for (auto& cpu : cpus) {
            if (!cpu.isAvailable() && cpu.getCurrentProcess().getName() == name) {
                Process& p = cpu.getCurrentProcess();
                clearScreen();

                std::string input;
                std::cout << "Process name: " << p.getName() << "\n";
                std::cout << "ID: " << p.assignedCore << "\n";
                std::cout << "Logs:\n";
                for (const auto& log : p.logs) {
                    std::cout << log << "\n";
                }
                std::cout << "\nCurrent instruction line: " << p.currentLine << "\n";
                std::cout << "Lines of code: " << p.getTotalLines() << "\n\n";

                do {
                    std::cout << "root:\\> ";
                    std::getline(std::cin, input);

                    if (input == "process-smi") {
                        std::cout << "Process name: " << p.getName() << "\n";
                        std::cout << "ID: " << p.assignedCore << "\n";
                        std::cout << "Logs:\n";
                        for (const auto& log : p.logs) {
                            std::cout << log << "\n";
                        }
                        std::cout << "\nCurrent instruction line: " << p.currentLine << "\n";
                        std::cout << "Lines of code: " << p.getTotalLines() << "\n";
                        if (p.status == "finished" || p.currentLine == p.getTotalLines()) {
                            std::cout << "\nFinished!\n";
                        }
                    }
                    else if (input != "exit") {
                        std::cout << "Unknown command\n";
                    }

                } while (input != "exit");

                clearScreen();
                return;
            }
        }

        // 2. Finished
        for (const auto& p : finishedProcesses) {
            if (p.getName() == name) {
                clearScreen();
                std::cout << "Process name: " << p.getName() << "\n";
                std::cout << "ID: " << p.assignedCore << "\n";
                std::cout << "Logs:\n";
                for (const auto& log : p.logs) {
                    std::cout << log << "\n";
                }
                std::cout << "\nCurrent instruction line: " << p.currentLine << "\n";
                std::cout << "Lines of code: " << p.getTotalLines() << "\n";
                std::cout << "\nFinished!\n";
                std::cout << "root:\\> ";
                std::string dummy;
                std::getline(std::cin, dummy);
                clearScreen();
                return;
            }
        }

        // 3. Queued
        for (auto& p : processQueue) {
            if (p.getName() == name) {
                clearScreen();
                std::cout << "Process name: " << p.getName() << "\n";
                std::cout << "ID: waiting\n";
                std::cout << "Logs:\n(No logs yet. Waiting to be scheduled...)\n";
                std::cout << "\nCurrent instruction line: " << p.currentLine << "\n";
                std::cout << "Lines of code: " << p.getTotalLines() << "\n";
                std::cout << "\nroot:\\> ";
                std::string dummy;
                std::getline(std::cin, dummy);
                clearScreen();
                return;
            }
        }

        std::cout << "Process \"" << name << "\" not found.\n";
    }
};

#endif // SCHEDULER_H
