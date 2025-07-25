#ifndef Pager_H
#define Pager_H

#include <iostream>
#include <string>
#include <unordered_map> // <-- Required for unordered_map
#include <utility>       // <-- Required for std::pair
#include "BackInStore.h"

class Pager {
public:
    // (FrameNumber) -> (ProcessName, PageNumber)
    std::unordered_map<int, std::pair<std::string, int>> pageTable;
    BackInStore* store;

    Pager(int totalFrames, BackInStore* store) : store(store) {
        for (int i = 0; i < totalFrames; ++i) {
            pageTable[i] = { "", -1 };
        }
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
            if (entry.second.first.empty()) {
                entry.second = { processName, pageNumber };
                std::cout << "Assigned Frame " << entry.first
                    << " to Process: " << processName
                    << ", Page: " << pageNumber << '\n';
                std::string pageIdentifier = processName + "_page_" + std::to_string(pageNumber);
                store->deleteEntry(pageIdentifier);
                return true;

            }
        }

        std::cout << "No empty frames available for Process: " << processName
            << ", Page: " << pageNumber << '\n';

        std::string pageIdentifier = processName + "_page_" + std::to_string(pageNumber);
        store->push(pageIdentifier);  

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

    // Checks if a specific page of a process is currently in memory
    bool hasPageInMemory(const std::string& processName, int pageNumber) const {
        for (const auto& entry : pageTable) {
            if (entry.second.first == processName && entry.second.second == pageNumber) {
                return true;
            }
        }
        return false;
    }





};


#endif // Pager_H
