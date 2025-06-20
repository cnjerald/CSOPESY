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
    int RRexecutionCounter = 0;

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
    void oneClockCycle() {
        if (!isIdle && assigned_process.currentLine < assigned_process.getTotalLines()) {
            assigned_process.executeInstruction();  // Executes any type        
        } else if (!isIdle && assigned_process.currentLine == assigned_process.getTotalLines()){
            assigned_process.setStatus("finished");
        }
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
        return assigned_process;  // still okay to return by value here
    }
};

#endif // CPU_H
