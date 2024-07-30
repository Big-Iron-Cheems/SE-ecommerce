#pragma once

#include "../Utils.h"
#include <memory>
#include <pqxx/pqxx>

/**
 * A singleton class that manages a pool of connections to Postgres databases
 */
class PostgresConnectionPool {
public:
    PostgresConnectionPool() = default;
    PostgresConnectionPool(const PostgresConnectionPool &) = delete;
    PostgresConnectionPool &operator=(const PostgresConnectionPool &) = delete;

    ~PostgresConnectionPool() { Utils::log(Utils::LogLevel::TRACE, std::cout, std::format("Closing {} Postgres connections...", connections.size())); }

    /**
     * Get the singleton instance of the PostgresConnectionPool class
     * @return The singleton instance of the PostgresConnectionPool class
     */
    static PostgresConnectionPool &getInstance();

    /**
     * Get a connection to a Postgres database
     * @param dbname The name of the database
     * @param user The username to use for the connection
     * @param password The password to use for the connection
     * @return The connection to the Postgres database
     * @throws std::runtime_error if the connection fails
     */
    std::shared_ptr<pqxx::connection> getConnection(const std::string &dbname, const std::string &user, const std::string &password);

private:
    std::unordered_map<std::string, std::shared_ptr<pqxx::connection>> connections;
    std::mutex mutex;
};
