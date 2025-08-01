#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <cmath>
#include <string>
#include "Process.h"

class CPU {
public:
    std::string cpu_name;
    Process assigned_process;
    int quantum_cycles;
    bool isIdle;
    int RRexecutionCounter = 0;
    int idleTicks = 0;
    int activeTicks = 0;
    int totalTicks = 0;

    // Constructor
    CPU(int cpu_number, int quantum_cycles)
        : cpu_name("CPU" + std::to_string(cpu_number)),
        quantum_cycles(quantum_cycles),
        isIdle(true) {
    }

    // Assign a process to the CPU
    void assignProcess(const Process& process) {
        assigned_process = process;
        assigned_process.assignedCore = std::stoi(cpu_name.substr(3));
        isIdle = false;
    }

    // Simulate one clock cycle
    void oneClockCycle(Pager& pager) {
        // Add to total ticks
        totalTicks++;
        if (isIdle) {
            idleTicks++;
        return;
        }
        // Add to active ticks if it gets pass idle
        activeTicks++;

        int currentLine = assigned_process.currentLine;
        int totalLines = assigned_process.getTotalLines();

        if (currentLine >= totalLines) {
            assigned_process.setStatus("finished");
            evictUnusedPages(pager);
            return;
        }

        // Ensure the page for currentLine is in memory
        if (!handlePageFault(pager)) {
            std::cout << "Page fault at line " << currentLine
                << " (Page #" << getPageIndexForLine(currentLine)
                << ") \n";
            return; // Wait for memory before executing
        }

        assigned_process.executeInstruction();


        // Evict any no-longer-needed pages after execution
        evictUnusedPages(pager);

    }

    void evictUnusedPages(Pager& pager) {
        const auto& instructions = assigned_process.instructions;
        auto& pages = assigned_process.pages;
        int currentLine = assigned_process.currentLine;
        const std::string& name = assigned_process.getName();

        int instructionsPerPage = std::ceil((float)instructions.size() / pages.size());

        for (int i = 0; i < pages.size(); ++i) {
            int startLine = i * instructionsPerPage;
            int endLine = std::min((i + 1) * instructionsPerPage, (int)instructions.size());

            if (pages[i].isValid && currentLine >= endLine) {
                pages[i].isValid = false;
                pager.removeFrame(name, i);
                std::cout << "Evicted page #" << i << " of process " << name << '\n';
            }
        }
    }

    bool handlePageFault(Pager& pager) {
        int currentLine = assigned_process.currentLine;
        int pageIndex = getPageIndexForLine(currentLine);
        if (!assigned_process.pages[pageIndex].isValid) {
            if (pager.assignFrame(assigned_process.getName(), pageIndex)) {
                assigned_process.pages[pageIndex].isValid = true;
                std::cout << "Handled page fault: Assigned Page #" << pageIndex << '\n';
                return true;
            }
            return false; // Still no free frame
        }
        return true; // Page already valid
    }

    int getPageIndexForLine(int line) const {
        int instructionsPerPage = std::ceil((float)assigned_process.instructions.size() / assigned_process.pages.size());
        return line / instructionsPerPage;
    }

    void printProcessTick() const {
    std::cout << "Total Ticks: " << totalTicks << "\n";
    std::cout << "Active Ticks: " << activeTicks << "\n";   
    std::cout << "Idle Ticks: " << idleTicks << "\n";
    }

    bool isAvailable() const {
        return isIdle;
    }

    Process& getCurrentProcess() {
        return assigned_process;
    }

    const Process& getCurrentProcess() const {
        return assigned_process;
    }

    bool isFinished() const {
        return !isIdle && assigned_process.status == "finished";
    }

    Process retrieveFinishedProcess() {
        isIdle = true;
        return assigned_process;  // safe to return by value
    }

    int getTotalTicks() const {
        return activeTicks + idleTicks;
    }

    int getActiveTicks() const {
        return activeTicks;
    }

    int getIdleTicks() const {
        return idleTicks;
    }


};

#endif // CPU_H
