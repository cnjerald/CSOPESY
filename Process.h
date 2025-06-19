#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <fstream>  // required for std::ofstream
#include <iostream> // for std::cerr
#include "Utils.h"

class Process {
public:
    std::string status;
    int currentLine;
    int assignedCore;
    std::vector<std::string> logs;

    Process()
        : name(""), time(""), totalLines(0),
        status("ready"), currentLine(0), assignedCore(-1) {
    }

    Process(const std::string& name, const std::string& time, int totalLines,
        const std::string& status = "ready", int currentLine = 0, int assignedCore = -1)
        : name(name), time(time), totalLines(totalLines),
        status(status), currentLine(currentLine), assignedCore(assignedCore) {
    }

    std::string getName() const { return name; }
    std::string getTime() const { return time; }
    int getTotalLines() const { return totalLines; }

    void printCommand() {
        std::string logEntry = getCurrentTime() + " Core:" + std::to_string(assignedCore)
                            + " Line: " + std::to_string(currentLine)
                            + " Hello world from " + name + "!";

        // ✅ Add to in-memory logs for screen output
        logs.push_back(logEntry);

        // ✅ Write to file for audit/logging
        std::ofstream file(name + ".txt", std::ios::app);
        if (file.is_open()) {
            file << logEntry << "\n";
            file.close();
        } else {
            std::cerr << "Failed to open file for process " << name << ".\n";
        }
    }

	void setStatus(const std::string& newStatus) {
		status = newStatus;
	}

private:
    std::string name;
    std::string time;
    int totalLines;
};

#endif // PROCESS_H
