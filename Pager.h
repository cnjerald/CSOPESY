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
    int pagesPagedIn = 0;
    int pagesPagedOut = 0;

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

        // First, try to find an empty frame
        for (auto& entry : pageTable) {
            if (entry.second.first.empty()) {
                entry.second = { processName, pageNumber };
                pagesPagedIn++;
                std::cout << "Assigned Frame " << entry.first
                    << " to Process: " << processName
                    << ", Page: " << pageNumber << '\n';
                std::string pageIdentifier = processName + "_page_" + std::to_string(pageNumber);
                store->deleteEntry(pageIdentifier);
                return true;
            }
        }

        // If no empty frame, evict one **belonging to the same process**
        for (auto& entry : pageTable) {
            if (entry.second.first == processName) {
                int evictPage = entry.second.second;
                std::cout << "Evicting page #" << evictPage << " of process " << processName << '\n';
                std::string evictIdentifier = processName + "_page_" + std::to_string(evictPage);
                store->push(evictIdentifier); // Save to store
                entry.second = { processName, pageNumber };
                std::cout << "Assigned Frame " << entry.first
                    << " to Process: " << processName
                    << ", Page: " << pageNumber << '\n';
                store->deleteEntry(processName + "_page_" + std::to_string(pageNumber));
                return true;
            }
        }


        std::cout << "No empty frames available for Process: " << processName
            << ", Page: " << pageNumber << '\n';

        return false;
    }





    void removeFrame(const std::string& processName, int pageNumber) {
        for (auto& entry : pageTable) {
            if (entry.second.first == processName && entry.second.second == pageNumber) {
                std::cout << "Evicted page #" << pageNumber << " of process " << processName << '\n';
                entry.second = { "", -1 };

                // Optional: notify process so it updates isValid
                // or set a callback or shared state
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

    int getUsedMemoryFrames() const {
        int used = 0;
        for (const auto& entry : pageTable) {
            if (!entry.second.first.empty()) {
                used++;
            }
        }
        return used;
    }

    int getTotalFrames() const {
        return pageTable.size();
    }


};


#endif // Pager_H
