#include "User.h"

std::string User::userTypeToString(User::UserType userType) {
    switch (userType) {
        case UserType::CUSTOMER: return "customer";
        case UserType::SUPPLIER: return "supplier";
        case UserType::TRANSPORTER: return "transporter";
        default: throw std::invalid_argument("Invalid user type");
    }
}

void User::openLogFile() {
    if (!logFile) logFile = std::make_shared<std::ofstream>(std::format("{}.log", userTypeToString(getUserType())), std::ios::out | std::ios::app);
}

void User::login() {
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
        std::string query = std::format("SELECT (check_user('{}', '{}')).*;", userType, name);
        pqxx::work tx(*conn);
        auto R = tx.query01<std::string, uint32_t, bool>(query);
        tx.commit();

        if (R) {
            // Access individual fields directly from the tuple
            auto [user_id, user_balance, logged_in] = R.value();

            // If the user is already in the database, fetch the id and balance, and set the logged_in field to true
            if (!logged_in) {
                id = user_id;

                query = std::format("SELECT set_logged_in('{}', {}, true);", userType, id);
                pqxx::work tx_login(*conn);
                tx_login.exec(query);
                tx_login.commit();
            } else throw std::invalid_argument("user already connected");
        } else {
            // Else, create a new entry in the database and fetch the id and balance, and set the logged_in field to true
            query = std::format("SELECT insert_user('{}', '{}');", userType, name);
            pqxx::work tx_new_user(*conn);
            auto new_user_id = tx_new_user.query_value<std::string>(query);
            tx_new_user.commit();

            id = new_user_id;
        }
        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("User `{}` logged in {{type: `{}`, id: {}, balance: {}}}", name, userType, id, getBalance()));
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
        std::string query = std::format("SELECT (check_user('{}', '{}')).logged_in;", userType, name);
        pqxx::work tx(*conn);
        bool logged_in = tx.query_value<bool>(query);
        tx.commit();

        if (logged_in) {
            // If the user's logged_in field is true, set the logged_in field to false
            query = std::format("SELECT set_logged_in('{}', {}, false);", userType, id);
            pqxx::work tx_logout(*conn);
            tx_logout.exec(query);
            tx_logout.commit();
            Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("User `{}` logged out", name));
        } else throw std::invalid_argument("User is not logged in");
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("An error occurred: {}", e.what()));
    }
}

uint32_t User::getBalance() const {
    std::string userType = userTypeToString(getUserType());
    try {
        // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", userType, userType);

        // Build the query
        std::string query = std::format("SELECT get_balance('{}', {});", userType, id);

        // Execute the query
        pqxx::work tx(*conn);
        auto bal = tx.query_value<uint32_t>(query);
        tx.commit();

        // Print the result
        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Balance: {}", bal));
        return bal;
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("An error occurred: {}", e.what()));
        return 0;
    }
}

void User::setBalance(const int32_t &balanceChange) {
    std::string userType = userTypeToString(getUserType());
    try {
        // Connect to the `ecommerce` database as the `userType` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", userType, userType);

        // Build the query to call the stored procedure
        std::string query = std::format("SELECT set_balance('{}', {}, {});", userType, id, balanceChange);

        // Execute the query
        pqxx::work tx(*conn);
        auto newBal = tx.query_value<uint32_t>(query);
        tx.commit();

        // Print the result
        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Balance modified to {}", newBal));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to set balance: {}", e.what()));
    }
}
