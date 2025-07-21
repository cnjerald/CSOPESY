#ifndef Pager_H
#define Pager_H

#include <iostream>
#include <string>
#include <unordered_map> // <-- Required for unordered_map
#include <utility>       // <-- Required for std::pair

class Pager {
public:
    // (FrameNumber) -> (ProcessName, PageNumber)
    std::unordered_map<int, std::pair<std::string, int>> pageTable;

    Pager(int totalFrames) {
        for (int i = 0; i < totalFrames; ++i) {
            pageTable[i] = { "", -1 }; // empty process name, invalid page
        }
        //std::cout << "Total frames: " << totalFrames << std::endl; //Debug

    }

    // Optional helper method to print the current page table
    void printPageTable() const {
        for (const auto& entry : pageTable) {
            std::cout << "Frame " << entry.first
                << " -> Process: " << (entry.second.first.empty() ? "None" : entry.second.first)
                << ", Page: " << entry.second.second << '\n';
        }
    }

    // Returns true if successfully allocated, false otherwise.
    bool assignFrame(const std::string& processName, int pageNumber) {
        for (auto& entry : pageTable) {
            if (entry.second.first.empty()) {  // empty frame
                entry.second = { processName, pageNumber };
                std::cout << "Assigned Frame " << entry.first
                    << " to Process: " << processName
                    << ", Page: " << pageNumber << '\n';
                return true;
            }
        }

        std::cout << "No empty frames available for Process: " << processName
            << ", Page: " << pageNumber << '\n';
        return false;
    }


    void removeFrame(const std::string& processName, int processPageNumber) {
        for (auto& entry : pageTable) {
            if (entry.second.first == processName && entry.second.second == processPageNumber) {
                entry.second = { "", -1 }; // Free the frame
                return;
            }
        }
    }




};


#endif // Pager_H
