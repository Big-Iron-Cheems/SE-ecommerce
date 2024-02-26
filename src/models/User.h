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

     explicit User(std::string name) : name(std::move(name)) {}

    /**
     * @return a string representation of the user.
     */
    [[maybe_unused]] [[nodiscard]] virtual std::string toString() const = 0;

    /**
     * Get the type of the user.
     * @return the type of the user.
     */
    [[nodiscard]] virtual std::string getUserType() const = 0;

    // Account related methods

    /**
     * Connect an instance of a user to the database.
     */
    virtual void connect2db() {
        //FIXME: add func to alter login status, correct checks using exec1 and catching, since if user is not present `SELECT loggedIn` throws an exception
        //TODO: make this actually return the id-balance pair so it can be set directly in the constructor
        std::string userType = getUserType();
        try {
            // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
            std::unique_ptr<pqxx::connection> conn = conn2Postgres("ecommerce", userType, userType);

            // Build the query
            std::string query = "SELECT loggedIn FROM " + userType + "s WHERE username = " + conn->quote(name) + ";";

            // Execute the query
            pqxx::work W(*conn);
            bool loggedIn = W.query_value<bool>(query);

            Utils::log(Utils::LogLevel::ALERT, std::cout, "Is logged in? ", loggedIn);
            // Check the result
            if (loggedIn) {
                throw std::runtime_error("User already connected to the database");
            } else {
                // query the database to check if the user is already in the database; note that the users are unique by their username
                query = "SELECT * FROM " + userType + "s WHERE username = " + conn->quote(name) + ";";
                pqxx::row R;

                try {
                    R = W.exec1(query);
                    // The user is already in the database, just connect and fetch the id and balance
                    id = R[0].as<uint32_t>();
                    balance = R[2].as<uint32_t>();
                    Utils::log(Utils::LogLevel::ALERT, std::cout, "User connected with id ", id, " and balance ", balance);
                } catch (const pqxx::unexpected_rows &e) {
                    // The user is not in the database, create a new entry
                    query = "INSERT INTO " + userType + "s (username, balance) VALUES (" + conn->quote(name) + ", " + conn->quote(0) + ") RETURNING id, balance;";
                    Utils::log(Utils::LogLevel::ALERT, std::cout, query);
                    R = W.exec1(query);

                    id = R[0].as<uint32_t>();
                    balance = R[2].as<uint32_t>();
                    Utils::log(Utils::LogLevel::ALERT, std::cout, "New user created with id ", id, " and balance ", 0);
                }
            }

            W.commit();
        } catch (const pqxx::broken_connection &e) {
            throw; // Rethrow the exception to propagate it to the caller
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to the database: ", e.what());
        }
    }

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
     */
    virtual void setBalance(const int32_t &balanceChange) {
        std::string userType = getUserType();
        try {
            // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
            std::unique_ptr<pqxx::connection> conn = conn2Postgres("ecommerce", userType, userType);

            // Build the query to call the stored procedure
            std::string query = "SELECT set_balance(" + conn->quote(userType) + ", " + std::to_string(id) + ", " + std::to_string(balanceChange) + ");";

            // Execute the query
            pqxx::work W(*conn);
            pqxx::result R = W.exec(query);
            W.commit();

            // Print the result
            if (!R.empty() && !R[0][0].is_null()) Utils::log(Utils::LogLevel::TRACE, std::cout, "Balance modified to ", R[0][0].as<std::string>());
            else Utils::log(Utils::LogLevel::TRACE, std::cout, "No balance modification performed");

        } catch (const pqxx::broken_connection &e) {
            throw; // Rethrow the exception to propagate it to the caller
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to set balance: ", e.what());
        }
    }
};
