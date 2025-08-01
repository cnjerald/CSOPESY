#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>  // <-- Required for ofstream]
#include <unordered_map>
#include <iomanip>
#include "Process.h"
#include <mutex>
#include "CPU.h"
#include "Pager.h"
#include "BackInStore.h"
#include "InitConfig.h"

class Scheduler {
private:
    Config config;

public:
    int num_cpu;
    int quantum_cycles;
    int delay_per_exec;
    std::string scheduler;
    std::deque<std::unique_ptr<Process>> processQueue;
    std::vector<Process> finishedProcesses;
    std::vector<CPU> cpus;
    std::mutex mtx;
    BackInStore store;
    Pager pager;

    Scheduler(int num_cpu, const std::string& scheduler, int quantum_cycles, int delay_per_exec, int total_frames)
        : num_cpu(num_cpu), scheduler(scheduler), quantum_cycles(quantum_cycles), delay_per_exec(delay_per_exec),
          store(), pager(total_frames, &store)
    {
        for (int i = 0; i < num_cpu; i++) {
            cpus.emplace_back(i, quantum_cycles);
        }
    }

    void setConfig(const Config& cfg) {
        config = cfg;
    }

    void checkQueue() {
        for (auto& cpu : cpus) {
            // Check if memory is available first before assigning to CPU (FCFS) no preemption soo..
			if (cpu.isAvailable() && !pager.pageTable.empty()) {
				// Check if any frame is available
				bool frameAvailable = false;
				for (const auto& entry : pager.pageTable) {
					if (entry.second.first.empty()) {  // Empty frame
						frameAvailable = true;
						break;
					}
				}
				if (!frameAvailable) {
					// std::cout << "No available frames for CPU " << cpu.cpu_name << ". Waiting for memory...\n";
					return;  // No available frames, skip this CPU
				}
			}
            
            if ((scheduler == "FCFS" || scheduler == "RR") && cpu.isAvailable() && !processQueue.empty()) {
                Process* process = processQueue.front().get();

                // Assign pages to frames before assigning to CPU
                for (int i = 0; i < process->pages.size(); ++i) {
                    if (!process->pages[i].isValid) {
                        bool success = pager.assignFrame(process->getName(), i);
                        if (success) {
                            process->pages[i].isValid = true;
                        }
                    }
                }
                
                // asign remaining pages of process->pages to backinstore.
                cpu.assignProcess(*process);  // Pass by reference
                processQueue.pop_front();     // Remove from queue after assignment
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

    void printAvailableMemory() {
        int totalFrames = 0;
        int usedFrames = 0;

        std::cout << "=== Frame Usage ===\n";

        for (const auto& entry : pager.pageTable) {
            int frameNumber = entry.first;
            const std::string& processName = entry.second.first;
            int pageNumber = entry.second.second;

            if (!processName.empty()) {
                std::cout << "Frame #" << frameNumber
                    << "  Process: " << processName
                    << ", Page: " << pageNumber << "\n";
                usedFrames++;
            }

            totalFrames++;  // Assuming all pageTable entries are valid frames
        }

        int availableFrames = totalFrames - usedFrames;

        std::cout << "\nSummary:\n";
        std::cout << "Total Frames: " << totalFrames << "\n";
        std::cout << "Used Frames: " << usedFrames << "\n";
        std::cout << "Available Frames: " << availableFrames << "\n";
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
        if (scheduler == "FCFS") {
            for (auto& cpu : cpus) {
                cpu.oneClockCycle(pager);
                if (cpu.isFinished()) {
                    Process finished = cpu.retrieveFinishedProcess();

                    // Purge memory used by this process
                    for (int i = 0; i < finished.pages.size(); ++i) {
                        if (finished.pages[i].isValid) {
                            pager.removeFrame(finished.getName(), i);
                        }
                    }

                    finishedProcesses.push_back(finished);
                }
            }
        }
        else if (scheduler == "RR") {
            for (auto& cpu : cpus) {
                try {
                    if (cpu.RRexecutionCounter <= cpu.quantum_cycles && !cpu.isIdle) {
                        cpu.oneClockCycle(pager);
                        cpu.RRexecutionCounter++;
                    }
                    else if (cpu.RRexecutionCounter > cpu.quantum_cycles && !cpu.isIdle) {
                        Process current = cpu.getCurrentProcess();

                        // Purge memory used by the current process
                        for (int i = 0; i < current.pages.size(); ++i) {
                            if (current.pages[i].isValid) {
                                pager.removeFrame(current.getName(), i);
                                current.pages[i].isValid = false;
                            }
                        }

                        // Requeue the modified process
                        processQueue.push_back(std::make_unique<Process>(current));
                        if (!processQueue.empty()) {

                            Process* process = processQueue.front().get();

                            // Ensure all required pages are loaded into memory
                            for (int i = 0; i < process->pages.size(); ++i) {
                                if (!process->pages[i].isValid) {
                                    bool success = pager.assignFrame(process->getName(), i);
                                    if (success) {
                                        process->pages[i].isValid = true;
                                    }
                                }
                            }

                            // Assign the process to the CPU and remove it from the queue
                            cpu.assignProcess(*process);
                            processQueue.pop_front();
                            cpu.RRexecutionCounter = 0;

                        }
                    }

                    if (cpu.isFinished()) {
                        Process finished = cpu.retrieveFinishedProcess();

                        // Purge memory used by this process
                        for (int i = 0; i < finished.pages.size(); ++i) {
                            if (finished.pages[i].isValid) {
                                pager.removeFrame(finished.getName(), i);
                            }
                        }

                        finishedProcesses.push_back(finished);
                        cpu.RRexecutionCounter = 0; // Reset
                    }
                }
                catch (const std::exception& e) {
                    // Handle error
                }
            }
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
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

    void printVMStat() {
        std::cout << "==== VM Statistics ====\n";

        // === Frame Usage ===
        int totalFrames = 0;
        int usedFrames = 0;

        std::cout << "\n=== Frame Usage ===\n";

        for (const auto& entry : pager.pageTable) {
            int frameNumber = entry.first;
            const std::string& processName = entry.second.first;
            int pageNumber = entry.second.second;

            if (!processName.empty()) {
                std::cout << "Frame #" << frameNumber
                        << "  Process: " << processName
                        << ", Page: " << pageNumber << "\n";
                usedFrames++;
            }

            totalFrames++;  // Assuming all pageTable entries are valid frames
        }

        int availableFrames = totalFrames - usedFrames;

        std::cout << "\n--- Frame Summary ---\n";
        std::cout << "Total Frames: " << totalFrames << "\n";
        std::cout << "Used Frames: " << usedFrames << "\n";
        std::cout << "Available Frames: " << availableFrames << "\n";

        // CPU Tick Stats
        int totalTicks = 0;
        int activeTicks = 0;
        int idleTicks = 0;

        for (const auto& cpu : cpus) {
            totalTicks += cpu.getTotalTicks();
            activeTicks += cpu.getActiveTicks();
            idleTicks += cpu.getIdleTicks();
        }

        std::cout << "\n=== CPU Tick Statistics ===\n";
        std::cout << "Total Ticks: " << totalTicks << "\n";
        std::cout << "Active Ticks: " << activeTicks << "\n";
        std::cout << "Idle Ticks: " << idleTicks << "\n";

        // Paging Stats
        std::cout << "\n=== Paging Statistics ===\n";
        std::cout << "Pages Paged In: " << pager.pagesPagedIn << "\n";
        std::cout << "Pages Paged Out: " << pager.pagesPagedOut << "\n";
    }

    void printProcessSMI() {
        std::cout << "========= process-smi =========\n";

        // --- CPU Utilization ---
        int idleCPUs = 0;
        for (const auto& cpu : cpus) {
            if (cpu.isIdle) idleCPUs++;
        }
        float cpuUtil = static_cast<float>(num_cpu - idleCPUs) / num_cpu * 100.0f;
        std::cout << "CPU Utilization: " << cpuUtil << "%\n";
        std::cout << "Cores Used: " << (num_cpu - idleCPUs) << " / " << num_cpu << "\n";

        // --- Memory Usage ---
        int maxMemoryBytes = config.max_overall_mem;
        int memPerFrame = config.mem_per_frame;

        int usedFrames = 0;
        for (const auto& entry : pager.pageTable) {
            if (!entry.second.first.empty()) {
                usedFrames++;
            }
        }

        int usedMemory = usedFrames * memPerFrame;
        float memUtil = static_cast<float>(usedMemory) / maxMemoryBytes * 100.0f;

        int freeMemory = maxMemoryBytes - usedMemory;

        std::cout << "Memory Utilization: " << memUtil << "%\n";
        std::cout << "Used Memory: " << usedMemory << " / " << maxMemoryBytes << " bytes\n";
        std::cout << "Free Memory: " << freeMemory << " bytes\n";

        std::cout << "===============================\n";
        std::cout << "\nRunning processes and memory usage:\n";

        // Map to hold memory usage per process
        std::unordered_map<std::string, int> processMemoryMap;

        for (const auto& entry : pager.pageTable) {
            const std::string& processName = entry.second.first;
            if (!processName.empty()) {
                processMemoryMap[processName] += memPerFrame;
            }
        }

        for (const auto& entry : processMemoryMap) {
            std::cout << entry.first << " " << entry.second << " bytes\n";
        }
    }
};

#endif // SCHEDULER_H
