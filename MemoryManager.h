// MemoryManager.h
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Utils.h"

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
        for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it) {
            if (!it->allocated && (it->end - it->start + 1) >= maxProcessMemory) {
                int originalStart = it->start;
                int originalEnd = it->end;

                // Modify the current block to be the allocated one
                it->end = originalStart + maxProcessMemory - 1;
                it->allocated = true;
                it->processName = processName;

                // Insert remaining free block after it (if any)
                if (it->end < originalEnd) {
                    MemoryBlock newBlock = { it->end + 1, originalEnd, false, "" };
                    memoryBlocks.insert(it + 1, newBlock);
                }

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

                // Try to merge with previous block
                if (it != memoryBlocks.begin()) {
                    auto prev = std::prev(it);
                    if (!prev->allocated) {
                        prev->end = it->end;
                        it = memoryBlocks.erase(it);  // erase current, move `it` to next
                        it = prev;  // move `it` back to prev (the merged block)
                    }
                }

                // Try to merge with next block
                if (std::next(it) != memoryBlocks.end()) {
                    auto next = std::next(it);
                    if (!next->allocated) {
                        it->end = next->end;
                        memoryBlocks.erase(next);
                    }
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

        std::filesystem::create_directory("memory_stamps");

        std::string filename = std::string("memory_stamps/") + "memory_stamp_" + std::to_string(quantum) + ".txt";

        std::ofstream outFile(filename, std::ios::app);

        if (outFile.is_open()) {
            outFile << "Timestamp: " << getCurrentTime() << "\n";

            // Count processes in memory
            int processCount = 0;
            for (const auto& block : memoryBlocks) {
                if (block.allocated) processCount++;
            }
            outFile << "Number of processes in memory: " << processCount << "\n";

            int fragmentation = calculateExternalFragmentation();
            outFile << "Total external fragmentation in KB: " << (fragmentation / 1024) << "\n\n";

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
        else {
            std::cout << ("Err opening file");
        }


        
    }
};

#endif // MEMORYMANAGER_H