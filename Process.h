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
        std::ofstream file(name + ".txt", std::ios::app);
        if (file.is_open()) {
			file << getCurrentTime() << " Core:" << assignedCore << " Line: " << currentLine << " Hello World from " << name << "!\n";
            file.close();
        }
        else {
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
