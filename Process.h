#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <cstdint>
#include <algorithm>
#include "Utils.h"
#include <filesystem>
#include <cmath>
#include "Pager.h"

struct Page {
    int pageNumber;
    bool isValid;
};

class Process {
public:
    std::string status;
    int currentLine;
    int assignedCore;
    std::vector<std::string> logs;
    std::vector<std::string> instructions;
    std::map<std::string, uint16_t> variables;
    int sleepCounter = 0;
    std::vector<Page> pages;

    Process()
        : name(""), time(""), totalLines(0),
        status("ready"), currentLine(0), assignedCore(-1) {
    }

    Process(const std::string& name, const std::string& time, int totalLines,
        int pageCount,
        const std::string& status = "ready", int currentLine = 0, int assignedCore = -1)
        : name(name), time(time), totalLines(totalLines),
        status(status), currentLine(currentLine), assignedCore(assignedCore) {

        generateRandomInstructions(totalLines);
        generateRandomPages(pageCount);
        //printPages();
    }

    std::string getName() const { return name; }
    std::string getTime() const { return time; }
    int getTotalLines() const { return totalLines; }

    void generateRandomPages(int pageCount) {
        for (int i = 0; i < pageCount; ++i) {
            pages.push_back(Page{ i, false }); // TEST Marked invalid initially
        }
    }
    void printPages() const {
        std::cout << "Process: " << name << " | Pages: \n";
        for (const auto& page : pages) {
            std::cout << "  Page #" << page.pageNumber
                << " | Valid: " << (page.isValid ? "Yes" : "No") << '\n';
        }
        std::cout << std::endl;
    }


    void printCommand() {
        std::string msg = "Hello world from " + name + "!";
        std::string logEntry = getCurrentTime() + " Core:" + std::to_string(assignedCore)
            + " " + msg;

        logs.push_back(logEntry);

        // Ensure the "logs" directory exists
        std::filesystem::create_directory("logs");

        std::string filePath = "logs/" + name + ".txt";
        std::ofstream file(filePath, std::ios::app);

        if (file.is_open()) {
            file << logEntry << "\n";
            file.close();
        }
        else {
            std::cerr << "Failed to open log file for process " << name << ".\n";
        }
    }

    void executeInstruction() {
        if (sleepCounter > 0) {
            sleepCounter--;
            return; // skip this cycle
        }

        if (currentLine >= instructions.size()) return;

        int instructionsPerPage = std::ceil((float)instructions.size() / pages.size());
        int pageIndex = currentLine / instructionsPerPage;

        if (pageIndex >= pages.size() || !pages[pageIndex].isValid) {
            return;
        }

        std::string instr = instructions[currentLine];

        if (instr.find("PRINT") == 0) {
            printCommand();
        }
        else if (instr.find("DECLARE") == 0) {
            // e.g. DECLARE(x, 10)
            std::string var = "x";  // Simulated single var
            uint16_t val = rand() % 65536;
            variables[var] = val;
        }
        else if (instr.find("ADD") == 0) {
            std::string var1 = "x";
            std::string var2 = "x";
            uint16_t val2 = getValue(var2);
            uint16_t val3 = rand() % 10;
            uint32_t result = val2 + val3;
            variables[var1] = std::min((uint32_t)65535, result);
        }
        else if (instr.find("SUBTRACT") == 0) {
            std::string var1 = "x";
            std::string var2 = "x";
            uint16_t val2 = getValue(var2);
            uint16_t val3 = rand() % 10;
            int32_t result = static_cast<int32_t>(val2) - static_cast<int32_t>(val3);
            variables[var1] = static_cast<uint16_t>(std::max(0, result));
        }
        else if (instr.find("SLEEP") == 0) {
            sleepCounter = 1 + (rand() % 3);  // SLEEP(1~3 ticks)
        }
        else if (instr.find("FOR") == 0) {
            // Remaining slots after this instruction
            int remainingLines = getTotalLines() - currentLine - 1;
            int maxInsert = std::min(remainingLines, 3);
            int loopCount = 1 + (rand() % (maxInsert + 1));  // Between 1 and maxInsert

            for (int i = 0; i < loopCount; i++) {
                currentLine++;
                instructions.insert(instructions.begin() + currentLine, "PRINT");
            }
            return;
        }
        currentLine++;
        return;

    }

    void setStatus(const std::string& newStatus) {
        status = newStatus;
    }

    void generateRandomInstructions(int count) {
        static std::vector<std::string> types = {
            "PRINT", "ADD", "SUBTRACT", "DECLARE", "SLEEP", "FOR"
        };
        instructions.clear();
        for (int i = 0; i < count; ++i) {
            int index = rand() % types.size();
            instructions.push_back(types[index]);
        }
    }



private:
    std::string name;
    std::string time;
    int totalLines;

    uint16_t getValue(const std::string& key) {
        if (variables.find(key) == variables.end()) {
            variables[key] = 0;
        }
        return variables[key];
    }
};

#endif // PROCESS_H
