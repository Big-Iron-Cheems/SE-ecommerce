#include "Utils.h"

void Utils::log(Utils::LogLevel level, std::ostream &ostream, const std::string &message) {
    std::string logPrefix;
    bool isOfstream = typeid(ostream) == typeid(std::ofstream); // If the given ostream is an ofstream, do not color the log message

    switch (level) {
        using enum Utils::LogLevel;
        using enum Utils::Color;
        case DEBUG: logPrefix = isOfstream ? "[DEBUG] " : std::format("{}[DEBUG] {}", color(YLW), color(RST)); break;
        case TRACE: logPrefix = isOfstream ? "[TRACE] " : std::format("{}[TRACE] {}", color(GRN), color(RST)); break;
        case ALERT: logPrefix = isOfstream ? "[ALERT] " : std::format("{}[ALERT] {}", color(MAG), color(RST)); break;
        case ERROR: logPrefix = isOfstream ? "[ERROR] " : std::format("{}[ERROR] {}", color(RED), color(RST)); break;
    }

    ostream << logPrefix << message << std::endl;
}
