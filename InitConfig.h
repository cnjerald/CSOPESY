#ifndef INITCONFIG_H
#define INITCONFIG_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct Config {
    int num_cpu;
    std::string scheduler;
    int quantum_cycles;
    int batch_process_freq;
    int min_ins;
    int max_ins;
    int delay_per_exec;
};




Config initConfig();

Config initConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file");
    }

    Config config;
    std::string key, value;

    while (file >> key) {
        file >> std::ws;
        if (file.peek() == '"') {
            file.get(); // remove opening quote
            std::getline(file, value, '"');
        }
        else {
            file >> value;
        }

        if (key == "num-cpu") config.num_cpu = std::stoi(value);
        else if (key == "scheduler") config.scheduler = value;
        else if (key == "quantum-cycles") config.quantum_cycles = std::stoi(value);
        else if (key == "batch-process-freq") config.batch_process_freq = std::stoi(value);
        else if (key == "min-ins") config.min_ins = std::stoi(value);
        else if (key == "max-ins") config.max_ins = std::stoi(value);
        else if (key == "delay-per-exec") config.delay_per_exec = std::stoi(value);
    }

    return config;
}

#endif