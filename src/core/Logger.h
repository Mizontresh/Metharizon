#pragma once
#include <iostream>
#include <string>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static void init();
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

private:
    static void log(Level level, const std::string& message);
    static std::string levelToString(Level level);
}; 