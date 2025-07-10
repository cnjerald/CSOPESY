#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>  // <-- Required for ofstream
#include "Process.h"
#include <mutex>
#include "CPU.h"
#include "MemoryManager.h"

class Scheduler {
private:
    MemoryManager memoryManager;
    int cycleCounter = 0;

public:
    // int cycleCounter = 0;
    int num_cpu;
    int quantum_cycles;
    int delay_per_exec;
    std::string scheduler;
    std::deque<std::unique_ptr<Process>> processQueue;
    std::vector<Process> finishedProcesses;
    std::vector<CPU> cpus;
    std::mutex mtx;
    // MemoryManager memoryManager;

    Scheduler(int num_cpu, const std::string& scheduler, int quantum_cycles, int delay_per_exec, int max_mem, int mem_per_frame, int mem_per_proc) 
    : num_cpu(num_cpu), scheduler(scheduler), quantum_cycles(quantum_cycles), delay_per_exec(delay_per_exec), memoryManager(max_mem, mem_per_frame, mem_per_proc), cycleCounter(0) {
        for (int i = 0; i < num_cpu; i++) {
            cpus.emplace_back(i, quantum_cycles);
        }
    }
    
    void checkQueue() {
        for (auto& cpu : cpus) {
            if ((scheduler == "FCFS" || scheduler == "RR") && cpu.isAvailable() && !processQueue.empty()) {
                auto& process = processQueue.front();
                
                // This will try to allocate memory to current process
                if (memoryManager.allocateMemory(process->getName())) {
                    cpu.assignProcess(*process);
                    processQueue.pop_front();
                } else {
                    // No memory available, move back to queue and repeat
                    processQueue.push_back(std::move(process));
                    processQueue.pop_front();
                }
            }
        }
    }

    void printProcessQueue() {
        int idleCPUs = 0;
        for (const auto& cpu : cpus) {
            if (cpu.isIdle) idleCPUs++;
        }
        float utilPercent = static_cast<float>(num_cpu - idleCPUs) / num_cpu * 100.0f;

        std::cout << "CPU Utilization: " << utilPercent << "%\n";
        std::cout << "Cores Used: " << (num_cpu - idleCPUs) << "\n";
        std::cout << "Cores Available: " << idleCPUs << "\n";
        std::cout << "==========================================\n";

        if (processQueue.empty()) {
            std::cout << "No processes in the queue.\n";
            return;
        }

        std::cout << "=== Current Process Queue ===\n";
        for (const auto& p : processQueue) {
            std::cout << "  Name: " << p->getName()
                << " | Time: " << p->getTime()
                << " | Progress: " << p->currentLine << "/" << p->getTotalLines()
                << " | Status: " << p->status << "\n";
        }
    }

    void printCurrentProcess() {
        std::cout << "=== Current Processes on CPUs ===\n";
        for (const auto& cpu : cpus) {
            if (!cpu.isAvailable()) {
                const Process& p = cpu.getCurrentProcess();
                std::cout << "Current process on " << cpu.cpu_name << ":"
                    << "  Name: " << p.getName()
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

    void printSystemStatusToFile() {
        std::ofstream outFile("system_status.txt");
        if (!outFile.is_open()) {
            std::cerr << "Failed to open file for writing.\n";
            return;
        }

        int idleCPUs = 0;
        for (const auto& cpu : cpus) {
            if (cpu.isIdle) idleCPUs++;
        }
        float utilPercent = static_cast<float>(num_cpu - idleCPUs) / num_cpu * 100.0f;

        outFile << "CPU Utilization: " << utilPercent << "%\n";
        outFile << "Cores Used: " << (num_cpu - idleCPUs) << "\n";
        outFile << "Cores Available: " << idleCPUs << "\n";
        outFile << "==========================================\n";

        if (processQueue.empty()) {
            outFile << "No processes in the queue.\n";
        }
        else {
            outFile << "=== Current Process Queue ===\n";
            for (const auto& p : processQueue) {
                outFile << "  Name: " << p->getName()
                    << " | Time: " << p->getTime()
                    << " | Progress: " << p->currentLine << "/" << p->getTotalLines()
                    << " | Status: " << p->status << "\n";
            }
        }

        outFile << "=== Current Processes on CPUs ===\n";
        for (const auto& cpu : cpus) {
            if (!cpu.isAvailable()) {
                const Process& p = cpu.getCurrentProcess();
                outFile << "Current process on " << cpu.cpu_name << ":"
                    << "  Name: " << p.getName()
                    << " | Time: " << p.getTime()
                    << " | Progress: " << p.currentLine << "/" << p.getTotalLines()
                    << " | Status: " << p.status << "\n";
            }
        }

        if (finishedProcesses.empty()) {
            outFile << "No finished processes.\n";
        }
        else {
            outFile << "=== Finished Processes ===\n";
            for (const auto& p : finishedProcesses) {
                outFile << "  Name: " << p.getName()
                    << " | Time: " << p.getTime()
                    << " | Progress: " << p.currentLine << "/" << p.getTotalLines()
                    << " | Status: " << p.status << "\n";
            }
        }

        outFile.close();
    }

    void addQueue(const Process& process) {
        std::lock_guard<std::mutex> lock(mtx);
        processQueue.push_back(std::make_unique<Process>(process));
        // Do NOT call checkQueue() here (handled by thread)
    }

    void runOneCycle() {
        if (scheduler == "RR") {
            for (auto& cpu : cpus) {
                try {
                    if (cpu.RRexecutionCounter <= quantum_cycles && !cpu.isIdle) {
                        cpu.oneClockCycle();
                        cpu.RRexecutionCounter++;
                        
                        // Generate snapshot every quantum cycle
                        if (cpu.RRexecutionCounter % quantum_cycles == 0) {
                            memoryManager.generateMemorySnapshot(cycleCounter);
                        }
                    }
                    else if (cpu.RRexecutionCounter > quantum_cycles && !cpu.isIdle) {
                        processQueue.push_back(std::make_unique<Process>(cpu.getCurrentProcess()));
                        if (!processQueue.empty()) {
                            cpu.assignProcess(*processQueue.front());
                            processQueue.pop_front();
                            cpu.RRexecutionCounter = 0;
                        }
                    }

                    if (cpu.isFinished()) {
                        Process finished = cpu.retrieveFinishedProcess();
                        finishedProcesses.push_back(finished);
                        memoryManager.deallocateMemory(finished.getName());
                        cpu.RRexecutionCounter = 0;
                    }
                }
                catch (const std::exception& e) {
                    // Handle error
                }
            }
            cycleCounter++;
        }
    }

    void runOneCycleLoop() {
        static int cycleCounter = 0;
        static int skipCount = 0;
        const int N = delay_per_exec;
        while (true) {
            if (skipCount < N) { ++skipCount; continue; }
            skipCount = 0;
            {
                std::lock_guard<std::mutex> lock(mtx); // LOCK
                runOneCycle();
            }
            ++cycleCounter;
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void checkQueueLoop() {
        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx); // LOCK
                checkQueue();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void attachToScreen(const std::string& name) {
        for (auto& cpu : cpus) {
            if (!cpu.isAvailable() && cpu.getCurrentProcess().getName() == name) {
                Process& p = cpu.getCurrentProcess();
                clearScreen();
                std::string input;
                std::cout << "Process name: " << p.getName() << "\n";
                std::cout << "ID: " << p.assignedCore << "\n";
                std::cout << "Logs:\n";
                for (const auto& log : p.logs) std::cout << log << "\n";
                std::cout << "\nCurrent instruction line: " << p.currentLine << "\n";
                std::cout << "Lines of code: " << p.getTotalLines() << "\n\n";

                do {
                    std::cout << "root:\\> ";
                    std::getline(std::cin, input);
                    if (input == "process-smi") {
                        std::cout << "Process name: " << p.getName() << "\n";
                        std::cout << "ID: " << p.assignedCore << "\n";
                        std::cout << "Logs:\n";
                        for (const auto& log : p.logs) std::cout << log << "\n";
                        std::cout << "\nCurrent instruction line: " << p.currentLine << "\n";
                        std::cout << "Lines of code: " << p.getTotalLines() << "\n";
                        if (p.status == "finished" || p.currentLine == p.getTotalLines())
                            std::cout << "\nFinished!\n";
                    }
                    else if (input != "exit") {
                        std::cout << "Unknown command\n";
                    }
                } while (input != "exit");

                clearScreen();
                return;
            }
        }

        for (const auto& p : finishedProcesses) {
            if (p.getName() == name) {
                clearScreen();
                std::cout << "Process name: " << p.getName() << "\n";
                std::cout << "ID: " << p.assignedCore << "\n";
                std::cout << "Logs:\n";
                for (const auto& log : p.logs) std::cout << log << "\n";
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

        for (const auto& p : processQueue) {
            if (p->getName() == name) {
                clearScreen();
                std::cout << "Process name: " << p->getName() << "\n";
                std::cout << "ID: waiting\n";
                std::cout << "Logs:\n(No logs yet. Waiting to be scheduled...)\n";
                std::cout << "\nCurrent instruction line: " << p->currentLine << "\n";
                std::cout << "Lines of code: " << p->getTotalLines() << "\n";
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
