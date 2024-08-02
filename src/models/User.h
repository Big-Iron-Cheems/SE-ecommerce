#pragma once

#include "../db/dbutils.h"
#include "../redis/rdutils.h"
#include <optional>

/**
 * Abstract class representing a user of the system.
 */
class User {
protected:
    std::shared_ptr<std::ofstream> logFile; ///< Log file for the user, shared among subclasses.

    std::string id = "0";              ///< Unique identifier of the user.
    std::string name;                  ///< Name of the user.
    uint32_t balance = 0;              ///< Balance of the user.
    bool loggedInSuccessfully = false; ///< Whether the user logged in successfully.

    explicit User(std::string name) : name(std::move(name)) {}

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

    /**
     * Open the log file for the user, if it is not already open.
     * The log file is named after the user type.
     * Multiple users of the same type use the same logger.
     */
    void openLogFile();

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
    virtual ~User() = default;

    // Balance related methods

    /**
     * Get the balance of the user.
     */
    [[maybe_unused]] void getBalance() const;

    /**
     * Set the balance of the user. Can add/remove money from the balance.
     * @param balanceChange the amount of money to add/remove from the balance.
     */
    [[maybe_unused]] void setBalance(const int32_t &balanceChange);
};
