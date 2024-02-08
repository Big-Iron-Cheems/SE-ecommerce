#pragma once

#include "model_utils.h"

/**
 * Abstract class representing a user of the system.
 */
class User {
protected:
    uint32_t id;      ///< Unique identifier of the user.
    std::string name; ///< Name of the user.
    uint32_t balance; ///< Balance of the user.

    User(std::string name, const uint32_t &balance = 0) : name(std::move(name)), balance(balance) {}

    /**
     * @return a string representation of the user.
     */
    [[maybe_unused]] [[nodiscard]] virtual std::string toString() const = 0;

    // Balance related methods

    /**
     * Get the balance of the user.
     */
    virtual void getBalance() const = 0;

    /**
     * Set the balance of the user. Can add/remove money from the balance.
     * @param balanceChange the amount of money to add/remove from the balance.
     * @param add whether to add or remove the money from the balance.
     */
    virtual void setBalance(const uint32_t &balanceChange, bool add) = 0;
};
