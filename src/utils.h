#pragma once

#include <iostream>
#include <map>
#include <sstream>

class Utils {
private:
    Utils() = default; // Private constructor

    /**
     * ANSI color codes.
     */
    enum class Color { RED, GRN, YLW, BLU, MAG, CYN, WHT, RST };

    /**
     * Returns the ANSI color code for the specified color.
     */
    constexpr static const char *color(Color color) {
        switch (color) {
            case Color::RED: return "\033[1;31m";
            case Color::GRN: return "\033[1;32m";
            case Color::YLW: return "\033[1;33m";
            case Color::BLU: return "\033[1;34m";
            case Color::MAG: return "\033[1;35m";
            case Color::CYN: return "\033[1;36m";
            case Color::WHT: return "\033[1;37m";
            case Color::RST: return "\033[0m";
        }
        return nullptr; // To silence compiler warning
    }

public:
    Utils(const Utils &) = delete; ///< Copy constructor - deleted

    Utils &operator=(const Utils &) = delete; ///< Assignment operator - deleted

    /**
     * Available logging levels.
     */
    enum class LogLevel { DEBUG, TRACE, ALERT, ERROR };

    /**
     * Prints a log message to the specified output stream with the specified debug level.
     * @param level the debug level
     * @param ostream the output stream
     * @param contents the contents to print
     */
    template<typename... Contents>
    [[maybe_unused]] static void log(Utils::LogLevel level, std::ostream &ostream, Contents... contents);
};

template<typename... Contents>
void Utils::log(Utils::LogLevel level, std::ostream &ostream, Contents... contents) {
    std::ostringstream oss;
    (oss << ... << contents);

    switch (level) {
        case Utils::LogLevel::DEBUG: ostream << color(Color::YLW) << "[DEBUG] " << color(Color::RST); break;
        case Utils::LogLevel::TRACE: ostream << color(Color::GRN) << "[TRACE] " << color(Color::RST); break;
        case Utils::LogLevel::ALERT: ostream << color(Color::MAG) << "[ALERT] " << color(Color::RST); break;
        case Utils::LogLevel::ERROR: ostream << color(Color::RED) << "[ERROR] " << color(Color::RST); break;
    }

    ostream << oss.str() << std::endl;
}
