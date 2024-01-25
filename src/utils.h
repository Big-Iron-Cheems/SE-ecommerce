#ifndef ECOMMERCE_UTILS_H
#define ECOMMERCE_UTILS_H

#include <iostream>


extern bool DEBUG_MODE; ///< Set this to false to disable debug features

/**
 * Prints a log message to the `std::cout` stream.
 * @param message the message to print
 */
[[maybe_unused]] void tracePrint(const std::string &message);

/**
 * Prints a debug message to the `std::cout` stream.
 * @param message the message to print
 */
[[maybe_unused]] void debugPrint(const std::string &message);

/**
 * Prints an error message to the `std::cerr` stream.
 * @param message the message to print
 */
[[maybe_unused]] void errorPrint(const std::string &message);

#endif //ECOMMERCE_UTILS_H
