// MemoryManager.h
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <string>
#include <fstream>
// #include <filesystem>

struct MemoryBlock {
    int start;
    int end;
    bool allocated;
    std::string processName;
};

class MemoryManager {
private:
    int totalMemory;
    int frameSize;
    int maxProcessMemory;
    std::vector<MemoryBlock> memoryBlocks;
    int currentQuantum = 0;

public:
    MemoryManager(int totalMem, int frameSz, int maxProcMem) 
        : totalMemory(totalMem), frameSize(frameSz), maxProcessMemory(maxProcMem) {
        // Initialize memory as one free block
        memoryBlocks.push_back({0, totalMemory - 1, false, ""});
    }

    bool allocateMemory(const std::string& processName) {
        // First-fit algorithm
        for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it) {
            if (!it->allocated && (it->end - it->start + 1) >= maxProcessMemory) {
                // Split the block if there's remaining space
                if ((it->end - it->start + 1) > maxProcessMemory) {
                    MemoryBlock newBlock = {it->start + maxProcessMemory, it->end, false, ""};
                    memoryBlocks.insert(it + 1, newBlock);
                }
                it->end = it->start + maxProcessMemory - 1;
                it->allocated = true;
                it->processName = processName;
                return true;
            }
        }
        return false; // No suitable block found
    }

    void deallocateMemory(const std::string& processName) {
        for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it) {
            if (it->allocated && it->processName == processName) {
                it->allocated = false;
                it->processName = "";
                
                // Merge with adjacent free blocks
                if (it != memoryBlocks.begin() && !(it-1)->allocated) {
                    (it-1)->end = it->end;
                    memoryBlocks.erase(it);
                    it = (it-1);
                }
                if (it != (memoryBlocks.end()-1) && !(it+1)->allocated) {
                    it->end = (it+1)->end;
                    memoryBlocks.erase(it+1);
                }
                break;
            }
        }
    }

    int calculateExternalFragmentation() {
        int totalFree = 0;
        int largestFreeBlock = 0;
        
        for (const auto& block : memoryBlocks) {
            if (!block.allocated) {
                int blockSize = block.end - block.start + 1;
                totalFree += blockSize;
                if (blockSize > largestFreeBlock) {
                    largestFreeBlock = blockSize;
                }
            }
        }
        
        // External fragmentation is total free memory that can't satisfy a new process
        return (largestFreeBlock >= maxProcessMemory) ? 0 : totalFree;
    }

    void generateMemorySnapshot(int quantum) {
        currentQuantum = quantum;

        // std::filesystem::create_directory("memory_stamps");
        // std::string filename = "memory_stamps/memory_stamp_" + std::to_string(quantum) + ".txt";
        std::string filename = "memory_stamp_" + std::to_string(quantum) + ".txt";
        std::ofstream outFile(filename);
        
        if (!outFile.is_open()) {
            return;
        }

        outFile << "Timestamp: " << getCurrentTime() << "\n";
        
        // Count processes currently in memory
        int processCount = 0;
        for (const auto& block : memoryBlocks) {
            if (block.allocated) processCount++;
        }
        outFile << "Number of processes in memory: " << processCount << "\n";
        
        int fragmentation = calculateExternalFragmentation();
        outFile << "Total external fragmentation in KB: " << (fragmentation/1024) << "\n\n";
        
        outFile << "---end--- = " << totalMemory << "\n\n";
        
        for (auto it = memoryBlocks.rbegin(); it != memoryBlocks.rend(); ++it) {
            if (it->allocated) {
                outFile << it->end + 1 << "\n";
                outFile << it->processName << "\n";
                outFile << it->start << "\n\n";
            }
        }
        
        outFile << "---start--- = 0\n";
        
        outFile.close();
    }

    // This function is in Utils.h but for some reason, the program stops working if I include "Utils.h"
    std::string getCurrentTime() {
        time_t timestamp;
        time(&timestamp);
        struct tm* localTime = localtime(&timestamp);

        char buffer[80];
        strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", localTime);

        return std::string(buffer);
    }

};

#endif // MEMORYMANAGER_H