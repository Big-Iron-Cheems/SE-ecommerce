#pragma once

#include "model_utils.h"

/**
 * Abstract class representing a user of the system.
 */
class User {
protected:
    std::string id;      ///< Unique identifier of the user.
    std::string name; ///< Name of the user.
    uint32_t balance; ///< Balance of the user.
    bool loggedInSuccessfully; ///< Whether the user logged in successfully.

    explicit User(std::string name) : id("0"), name(std::move(name)), balance(0), loggedInSuccessfully(false) {}

    enum class UserType { CUSTOMER, SUPPLIER, TRANSPORTER };

    /**
     * @return a string representation of the user.
     */
    [[maybe_unused]] [[nodiscard]] virtual std::string toString() const = 0;

    /**
     * Get the type of the user.
     * @return the type of the user.
     */
    [[nodiscard]] virtual UserType getUserType() const = 0;

    /**
     * Get the type of the user as a string.
     * @param userType the type of the user.
     * @return the string representation of the user type.
     */
    static std::string userTypeToString(UserType userType);

    // Account related methods

    /**
     * Connect an instance of a user to the database.
     */
    void login();

    /**
     * Disconnect an instance of a user from the database.
     */
    void logout();

public:
    // Balance related methods

    /**
     * Get the balance of the user.
     */
    void getBalance() const;

    /**
     * Set the balance of the user. Can add/remove money from the balance.
     * @param balanceChange the amount of money to add/remove from the balance.
     */
    void setBalance(const int32_t &balanceChange);
};
