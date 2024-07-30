#include "PostgresConnectionPool.h"

PostgresConnectionPool &PostgresConnectionPool::getInstance() {
    static PostgresConnectionPool instance;
    return instance;
}

std::shared_ptr<pqxx::connection> PostgresConnectionPool::getConnection(const std::string &dbname, const std::string &user, const std::string &password) {
    // Use a lock to ensure thread-safe access to the connection map
    std::lock_guard<std::mutex> lock(mutex);

    // Check if the connection already exists
    std::string connInfo = std::format("dbname={} user={} password={}", dbname, user, password);
    auto it = connections.find(connInfo);
    if (it != connections.end()) return it->second;

    // Create a new connection if it doesn't exist in the map, otherwise return the existing connection
    try {
        auto conn = std::make_shared<pqxx::connection>(connInfo);
        Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Connected to Postgres database '{}' as user '{}'.", dbname, user));
        connections[connInfo] = conn;
        return conn;
    } catch (const pqxx::broken_connection &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to connect to Postgres database '{}' as user '{}': {}", dbname, user, e.what()));
        throw std::runtime_error(std::format("Failed to connect to Postgres database '{}' as user '{}': {}", dbname, user, e.what()));
    }
}
