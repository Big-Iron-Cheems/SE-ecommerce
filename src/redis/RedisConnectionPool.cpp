#include "RedisConnectionPool.h"

RedisConnectionPool &RedisConnectionPool::getInstance() {
    static RedisConnectionPool instance;
    return instance;
}

std::shared_ptr<sw::redis::Redis> RedisConnectionPool::getConnection(const std::string &uri) {
    // Use a lock to ensure thread-safe access to the connection map
    std::lock_guard<std::mutex> lock(mutex);

    // Check if the connection already exists
    auto it = connections.find(uri);
    if (it != connections.end()) return it->second;

    // Create a new connection if it doesn't exist in the map, otherwise return the existing connection
    try {
        auto conn = std::make_shared<sw::redis::Redis>(uri);
        connections[uri] = conn;
        return conn;
    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to connect to Redis at URI `{}`: {}", uri, e.what()));
        throw std::runtime_error(std::format("Failed to connect to Redis at URI `{}`: {}", uri, e.what()));
    }
}
