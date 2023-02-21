//
// Created by Zay on 2/19/2023.
//

#include <functional>
#include <ctime>
#include <chrono>
#include <iostream>
#include "ConsoleLogger.h"

using namespace std;

void ConsoleLogger::Info(const char *content, ...) {
    time_t curr_time;
    curr_time = time(NULL);
    tm *tm_local = localtime(&curr_time);
    std::cout  << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec << " | INFO | " << content << "\n";
}
void ConsoleLogger::Debug(const char *content, ...) {
    time_t curr_time;
    curr_time = time(NULL);
    tm *tm_local = localtime(&curr_time);
    std::cout  << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec  << " | DEBUG | " << content << "\n";
}
void ConsoleLogger::Error(const char *content, ...) {
    time_t curr_time;
    curr_time = time(NULL);
    tm *tm_local = localtime(&curr_time);
    std::cout  << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec  << " | ERROR | " << content << "\n";
}
void ConsoleLogger::Warn(const char *content, ...) {
    time_t curr_time;
    curr_time = time(NULL);
    tm *tm_local = localtime(&curr_time);
    std::cout  << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec << " | WARN | " << content << "\n";
}
void ConsoleLogger::Critical(const char *content, ...) {
    time_t curr_time;
    curr_time = time(NULL);
    tm *tm_local = localtime(&curr_time);
    std::cout  << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec << " | CRIT | " << content << "\n";
}
void ConsoleLogger::Trace(const char *content, ...) {
    time_t curr_time;
    curr_time = time(NULL);
    tm *tm_local = localtime(&curr_time);
    std::cout  << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec << " | TRACE | " << content << "\n";
}
