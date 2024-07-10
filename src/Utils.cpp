#include "Utils.h"

void Utils::log(Utils::LogLevel level, std::ostream &ostream, const std::string &message) {
    std::string logPrefix;
    switch (level) {
        using enum Utils::LogLevel;
        using enum Utils::Color;
        case DEBUG: logPrefix = std::format("{}[DEBUG] {}", color(YLW), color(RST)); break;
        case TRACE: logPrefix = std::format("{}[TRACE] {}", color(GRN), color(RST)); break;
        case ALERT: logPrefix = std::format("{}[ALERT] {}", color(MAG), color(RST)); break;
        case ERROR: logPrefix = std::format("{}[ERROR] {}", color(RED), color(RST)); break;
    }

    ostream << logPrefix << message << std::endl;
}
