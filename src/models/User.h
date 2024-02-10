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

    /**
     * Get the type of the user.
     * @return the type of the user.
     */
    [[nodiscard]] virtual std::string getUserType() const = 0;

    // Balance related methods

    /**
     * Get the balance of the user.
     */
    virtual void getBalance() const {
        std::string userType = getUserType();
        try {
            // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
            std::unique_ptr<pqxx::connection> conn = conn2Postgres("ecommerce", userType, userType);

            // Build the query
            std::string query = "SELECT balance FROM " + userType + "s WHERE id = " + std::to_string(id) + ";";

            // Execute the query
            pqxx::work W(*conn);
            pqxx::result R = W.exec(query);
            W.commit();

            // Print the result
            if (!R.empty()) Utils::log(Utils::LogLevel::TRACE, std::cout, "Balance: ", R[0][0].as<std::string>());
        } catch (const pqxx::broken_connection &e) {
            throw; // Rethrow the exception to propagate it to the caller
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "An error occurred: ", e.what());
        }
    }

    /**
     * Set the balance of the user. Can add/remove money from the balance.
     * @param balanceChange the amount of money to add/remove from the balance.
     * @param add whether to add or remove the money from the balance.
     */
    virtual void setBalance(const uint32_t &balanceChange, bool add) {
        std::string userType = getUserType();
        try {
            // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
            std::unique_ptr<pqxx::connection> conn = conn2Postgres("ecommerce", userType, userType);

            // Build the query to call the stored procedure
            std::string query = "SELECT set_balance_" + userType + "(" + std::to_string(id) + ", " + std::to_string(balanceChange) + ", " + (add ? "TRUE" : "FALSE") + ");";

            // Execute the query
            pqxx::work W(*conn);
            pqxx::result R = W.exec(query);
            W.commit();

            // Print the result
            if (!R.empty()) Utils::log(Utils::LogLevel::TRACE, std::cout, (add ? "Balance increased to " : "Balance decreased to "), R[0][0].as<std::string>());
        } catch (const pqxx::broken_connection &e) {
            throw; // Rethrow the exception to propagate it to the caller
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to set balance: ", e.what());
        }
    }
};
