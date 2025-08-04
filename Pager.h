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
        // First check if this page is already loaded
        for (auto& entry : pageTable) {
            if (entry.second.first == processName && entry.second.second == pageNumber) {
                return true; // Page already in memory
            }
        }

        // Find existing frame assigned to this process (there should be exactly one)
        int existingFrame = -1;
        for (auto& entry : pageTable) {
            if (entry.second.first == processName) {
                existingFrame = entry.first;
                break;
            }
        }

        if (existingFrame != -1) {
            // Evict the current page and load the new one
            int evictPage = pageTable[existingFrame].second;
            std::string evictIdentifier = processName + "_page_" + std::to_string(evictPage);
            store->push(evictIdentifier);
            
            pageTable[existingFrame] = {processName, pageNumber};
            store->deleteEntry(processName + "_page_" + std::to_string(pageNumber));
            pagesPagedIn++;
            pagesPagedOut++;
            return true;
        }

        // Try to find any empty frame
        for (auto& entry : pageTable) {
            if (entry.second.first.empty()) {
                entry.second = {processName, pageNumber};
                pagesPagedIn++;
                store->deleteEntry(processName + "_page_" + std::to_string(pageNumber));
                return true;
            }
        }

        return false; // No available frames
    }

    void removeFrame(const std::string& processName, int pageNumber) {
        for (auto& entry : pageTable) {
            if (entry.second.first == processName && entry.second.second == pageNumber) {
                // std::cout << "Evicted page #" << pageNumber << " of process " << processName << '\n';
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
