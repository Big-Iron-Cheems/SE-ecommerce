#include "User.h"

std::string User::userTypeToString(User::UserType userType) {
    switch (userType) {
        case UserType::CUSTOMER: return "customer";
        case UserType::SUPPLIER: return "supplier";
        case UserType::TRANSPORTER: return "transporter";
        default: throw std::invalid_argument("Invalid user type");
    }
}

void User::login() {
    //TODO: maybe make this return the id-balance pair so it can be set directly in the constructor
    /*
     * This function should establish a connection to the db to allow creating an instance of a User subclass.
     * In order, it should:
     * 1. Connect to the `ecommerce` database as the `userType` user using conn2Postgres
     * 2. Check if the given username is already in the database
     *  2.1 If it is, fetch the id and balance, and set the logged_in field to true
     *  2.2 If it is not, create a new entry in the database and fetch the id and balance, and set the logged_in field to true
     *  2.3 If the user is already connected to the database, throw an exception
     * 3. If no exception is thrown, set the id and balance fields of the User subclass
     */
    std::string userType = userTypeToString(getUserType());
    try {
        // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", userType, userType);

        // Build the query to check if the user is already in the database
        std::string query = "SELECT id, balance, logged_in FROM " + userType + "s WHERE username = " + conn->quote(name) + ";";
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (!R.empty()) {
            // If the user is already in the database, fetch the id and balance, and set the logged_in field to true
            // Otherwise, throw an exception as the user is already connected
            if (!R[0][2].as<bool>()) {
                id = R[0][0].as<uint32_t>();
                balance = R[0][1].as<uint32_t>();

                query = "SELECT set_logged_in(" + conn->quote(userType) + ", " + std::to_string(id) + ", true);";
                pqxx::work tx_login(*conn);
                tx_login.exec(query);
                tx_login.commit();
            } else throw std::invalid_argument("user already connected");
        } else {
            // Else, create a new entry in the database and fetch the id and balance, and set the logged_in field to true
            query = "SELECT insert_user(" + conn->quote(userType) + ", " + conn->quote(name) + ");";
            pqxx::work tx_new_user(*conn);
            R = tx_new_user.exec(query);
            tx_new_user.commit();

            id = R[0][0].as<uint32_t>();
            balance = 0;
        }
        Utils::log(Utils::LogLevel::TRACE, std::cout, "User `", name, "` logged in as ", userType, " with id ", id, " and balance ", balance);
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        throw; // Rethrow the exception to propagate it to the caller
    }
}

void User::logout() {
    /*
     * This function should disconnect an instance of a user from the database.
     * In order, it should:
     *  1. Connect to the `ecommerce` database as the `userType` user using conn2Postgres
     *  2. Check if the user's logged_in field is true
     *   2.1 If it is, set the logged_in field to false
     *   2.2 If it is not, throw an exception
     */
    std::string userType = userTypeToString(getUserType());
    try {
        // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", userType, userType);

        // Build the query to check if the user's logged_in field is true
        std::string query = "SELECT logged_in FROM " + userType + "s WHERE id = " + std::to_string(id) + ";";
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R[0][0].as<bool>()) {
            // If the user's logged_in field is true, set the logged_in field to false
            query = "SELECT set_logged_in(" + conn->quote(userType) + ", " + std::to_string(id) + ", false);";
            pqxx::work tx_logout(*conn);
            tx_logout.exec(query);
            tx_logout.commit();
            Utils::log(Utils::LogLevel::TRACE, std::cout, "User `", name, "` logged out");
        } else throw std::invalid_argument("User is not logged in");
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "An error occurred: ", e.what());
    }
}

void User::getBalance() const {
    std::string userType = userTypeToString(getUserType());
    try {
        // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", userType, userType);

        // Build the query
        std::string query = "SELECT balance FROM " + userType + "s WHERE id = " + std::to_string(id) + ";";

        // Execute the query
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        // Print the result
        if (!R.empty()) Utils::log(Utils::LogLevel::TRACE, std::cout, "Balance: ", R[0][0].as<std::string>());
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "An error occurred: ", e.what());
    }
}

void User::setBalance(const int32_t &balanceChange) {
    std::string userType = userTypeToString(getUserType());
    try {
        // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", userType, userType);

        // Build the query to call the stored procedure
        std::string query = "SELECT set_balance(" + conn->quote(userType) + ", " + std::to_string(id) + ", " + std::to_string(balanceChange) + ");";

        // Execute the query
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        // Print the result
        if (!R.empty() && !R[0][0].is_null()) Utils::log(Utils::LogLevel::TRACE, std::cout, "Balance modified to ", R[0][0].as<std::string>());
        else Utils::log(Utils::LogLevel::TRACE, std::cout, "No balance modification performed");

    } catch (const pqxx::sql_error &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "SQL error: ", e.what(), "Query: ", e.query());
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to set balance: ", e.what());
    }
}
