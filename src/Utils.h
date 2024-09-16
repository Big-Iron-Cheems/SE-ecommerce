#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <utility>

class Utils {
private:
    /**
     * ANSI color codes.
     */
    enum class Color { RED, GRN, YLW, BLU, MAG, CYN, WHT, RST };

    /**
     * Returns the ANSI color code for the specified color.
     */
    constexpr static std::string color(Color color) {
        switch (color) {
            using enum Utils::Color;
            case RED: return "\033[1;31m";
            case GRN: return "\033[1;32m";
            case YLW: return "\033[1;33m";
            case BLU: return "\033[1;34m";
            case MAG: return "\033[1;35m";
            case CYN: return "\033[1;36m";
            case WHT: return "\033[1;37m";
            case RST: return "\033[0m";
        }
        std::unreachable(); // To silence compiler warning
    }

public:
    Utils() = delete;                              ///< Default constructor - deleted
    Utils(const Utils &other) = delete;            ///< Copy constructor - deleted
    Utils(Utils &&other) = delete;                 ///< Move constructor - deleted
    Utils &operator=(const Utils &other) = delete; ///< Copy assignment operator - deleted
    Utils &operator=(Utils &&other) = delete;      ///< Move assignment operator - deleted
    ~Utils() = delete;                             ///< Destructor - deleted


    /**
     * Available logging levels.
     */
    enum class LogLevel { DEBUG, TRACE, ALERT, ERROR };

    /**
     * Whether to log to console. Enabled via -v flag.
     */
    static bool logToConsole;

    /**
     * Prints a log message to the specified output stream with the specified debug level.
     * @param level the debug level
     * @param ostream the output stream
     * @param message the message to print, `std::format` for readability
     */
    static void log(Utils::LogLevel level, std::ostream &ostream, const std::string &message);
};
