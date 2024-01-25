#ifndef ECOMMERCE_USER_H
#define ECOMMERCE_USER_H

#include "model_utils.h"

/**
 * Abstract class representing a user of the system.
 */
class User {
protected:
    uint32_t id; ///< Unique identifier of the user.
    std::string name; ///< Name of the user.
    uint32_t balance; ///< Balance of the user.

    User(const uint32_t &id, std::string name, const uint32_t &balance = 0) : id(id), name(std::move(name)), balance(balance) {
    }

    /**
     * @return a string representation of the user.
     */
    [[maybe_unused]] [[nodiscard]] virtual std::string toString() const = 0;
};


#endif //ECOMMERCE_USER_H
