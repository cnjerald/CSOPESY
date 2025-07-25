#ifndef BACK_IN_STORE_H
#define BACK_IN_STORE_H

#include <iostream>
#include <fstream>
#include <string>
#include <deque>

class BackInStore {
private:
    std::string filePath;

public:
    // Constructor just creates/resets store.txt in the current directory
    BackInStore()
        : filePath("store.txt") {
        initialize();
    }

    // Create or reset the store.txt file
    void initialize() {
        std::ofstream file(filePath); // Truncates existing content
        if (file) {
            std::cout << "store.txt file initialized in root directory.\n";
        }
        else {
            std::cerr << "Failed to create store.txt\n";
        }
        file.close();
    }

    // Push a new entry in the format STRING,\n
    void push(const std::string& entry) {
        std::ofstream file(filePath, std::ios::app);
        if (!file) {
            std::cerr << "Failed to open store.txt for writing.\n";
            return;
        }
        file << entry << ",\n";
        file.close();
    }

    // Delete a specific entry from store.txt
    void deleteEntry(const std::string& target) {
        std::ifstream infile(filePath);
        if (!infile) {
            std::cerr << "Failed to open store.txt for reading.\n";
            return;
        }

        std::deque<std::string> entries;
        std::string line;
        std::string formattedTarget = target + ",";

        while (std::getline(infile, line)) {
            if (line != formattedTarget && !line.empty()) {
                entries.push_back(line);
            }
        }
        infile.close();

        std::ofstream outfile(filePath, std::ios::trunc);
        if (!outfile) {
            std::cerr << "Failed to open store.txt for writing during deletion.\n";
            return;
        }

        for (const auto& remaining : entries) {
            outfile << remaining << '\n';
        }

        outfile.close();
    }


    // Pop and return the first entry (FIFO)
    std::string pop() {
        std::ifstream infile(filePath);
        if (!infile) {
            std::cerr << "Failed to open store.txt for reading.\n";
            return "";
        }

        std::deque<std::string> entries;
        std::string line;

        while (std::getline(infile, line)) {
            if (!line.empty()) entries.push_back(line);
        }
        infile.close();

        if (entries.empty()) return "";

        std::string firstEntry = entries.front();
        entries.pop_front();

        std::ofstream outfile(filePath, std::ios::trunc);
        for (const auto& remaining : entries) {
            outfile << remaining << '\n';
        }
        outfile.close();

        // Remove trailing comma
        if (!firstEntry.empty() && firstEntry.back() == ',') {
            firstEntry.pop_back();
        }

        return firstEntry;
    }

    // Optional: check if store is empty
    bool isEmpty() const {
        std::ifstream file(filePath);
        return file.peek() == std::ifstream::traits_type::eof();
    }

    // Optional: print contents
    void printStore() const {
        std::ifstream file(filePath);
        std::string line;
        std::cout << "Contents of store.txt:\n";
        while (std::getline(file, line)) {
            std::cout << line << '\n';
        }
    }
};

#endif // BACK_IN_STORE_H
