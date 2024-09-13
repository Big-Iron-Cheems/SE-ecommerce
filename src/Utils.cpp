#include "Utils.h"

bool Utils::logToConsole = false;

void Utils::log(Utils::LogLevel level, std::ostream &ostream, const std::string &message) {
    std::string logPrefix;
    std::string coloredLogPrefix;
    bool isOfstream = typeid(ostream) == typeid(std::ofstream); // If the given ostream is an ofstream, do not color the log message

    switch (level) {
        using enum Utils::LogLevel;
        using enum Utils::Color;
        case DEBUG:
            logPrefix = "[DEBUG] ";
            coloredLogPrefix = std::format("{}[DEBUG] {}", color(YLW), color(RST));
            break;
        case TRACE:
            logPrefix = "[TRACE] ";
            coloredLogPrefix = std::format("{}[TRACE] {}", color(GRN), color(RST));
            break;
        case ALERT:
            logPrefix = "[ALERT] ";
            coloredLogPrefix = std::format("{}[ALERT] {}", color(MAG), color(RST));
            break;
        case ERROR:
            logPrefix = "[ERROR] ";
            coloredLogPrefix = std::format("{}[ERROR] {}", color(RED), color(RST));
            break;
    }

    ostream << (isOfstream ? logPrefix : coloredLogPrefix) << message << std::endl;

    if (isOfstream && logToConsole) {
        std::cout << coloredLogPrefix << message << std::endl;
    }
}
