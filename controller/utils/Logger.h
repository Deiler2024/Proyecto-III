#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
public:
    static void Info(const std::string& msg);
    static void Error(const std::string& msg);
};

#endif // LOGGER_H
