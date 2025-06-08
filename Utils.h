#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>

void clearScreen();
void printHeader();
std::string getCurrentTime();

void clearScreen() {
	#ifdef _WIN32
		std::system("cls");
	#else
		std::system("clear");
	#endif
}

void printHeader() {
	std::cout << "░█████╗░░██████╗░█████╗░██████╗░███████╗░██████╗██╗░░░██╗\n";
	std::cout << "██╔══██╗██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝╚██╗░██╔╝\n";
	std::cout << "██║░░╚═╝╚█████╗░██║░░██║██████╔╝█████╗░░╚█████╗░░╚████╔╝░\n";
	std::cout << "██║░░██╗░╚═══██╗██║░░██║██╔═══╝░██╔══╝░░░╚═══██╗░░╚██╔╝░░\n";
	std::cout << "╚█████╔╝██████╔╝╚█████╔╝██║░░░░░███████╗██████╔╝░░░██║░░░\n";
	std::cout << "\033[32mHello, Welcome to CSOPESY commandline!\033[0m\n";
	std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen \033[0m\n";
}

std::string getCurrentTime() {
	time_t timestamp;
	time(&timestamp);
	struct tm* localTime = localtime(&timestamp);

	char buffer[80];
	strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", localTime);

	return std::string(buffer);
}

#endif