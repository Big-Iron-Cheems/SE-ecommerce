#include "utils.h"

bool DEBUG_MODE = true;

[[maybe_unused]] void tracePrint(const std::string &message) {
    std::cout << "\033[1;32m" << "[TRACE] " << "\033[0m" << message << std::endl;
}

[[maybe_unused]] void debugPrint(const std::string &message) {
    if (DEBUG_MODE) {
        std::cout << "\033[1;33m" << "[DEBUG] " << "\033[0m" << message << std::endl;
    }
}

[[maybe_unused]] void errorPrint(const std::string &message) {
    std::cout << "\033[1;31m" << "[ERROR] " << "\033[0m" << message << std::endl;
}
