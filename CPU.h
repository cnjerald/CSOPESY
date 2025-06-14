#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <string>
#include "Process.h"

class CPU {
public:
    std::string cpu_name;
    Process assigned_process;
    int quantum_cycles;
    bool isIdle;

    // Constructor
    CPU(int cpu_number, int quantum_cycles)
        : cpu_name("CPU" + std::to_string(cpu_number)),
        quantum_cycles(quantum_cycles),
        isIdle(true) {
    }

    // Assign a process to the CPU
    void assignProcess(const Process& process) {
        assigned_process = process;
		assigned_process.assignedCore = std::stoi(cpu_name.substr(3)); // Extract CPU number from name
        isIdle = false;
    }

    // Simulate one clock cycle
    void oneClockCycle() {
        if (!isIdle && assigned_process.currentLine < assigned_process.getTotalLines()) {
            // Simulate processing ASSUMING PRINT COMMAND!
			assigned_process.printCommand();
            assigned_process.currentLine++;

            if (assigned_process.currentLine == assigned_process.getTotalLines()) {
                assigned_process.setStatus("finished");
            }
        }
    }

    // Check if CPU is idle
    bool isAvailable() const {
        return isIdle;
    }

    // Retrieve current process (optional)
    Process getCurrentProcess() const {
        return assigned_process;
    }

    bool isFinished() const {
        return !isIdle && assigned_process.status == "finished";
    }

    Process retrieveFinishedProcess() {
        isIdle = true;  // Mark CPU as available
        return assigned_process;
    }
};

#endif // CPU_H
