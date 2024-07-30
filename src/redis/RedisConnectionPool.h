#pragma once

#include "../Utils.h"
#include <memory>
#include <sw/redis++/redis++.h>

/**
 * A singleton class that manages a pool of connections to Redis servers
 */
class RedisConnectionPool {
public:
    RedisConnectionPool() = default;
    RedisConnectionPool(const RedisConnectionPool &) = delete;
    RedisConnectionPool &operator=(const RedisConnectionPool &) = delete;

    ~RedisConnectionPool() { Utils::log(Utils::LogLevel::TRACE, std::cout, std::format("Closing {} Redis connections...", connections.size())); }

    /**
     * Get the singleton instance of the RedisConnectionPool class
     * @return The singleton instance of the RedisConnectionPool class
     */
    static RedisConnectionPool &getInstance();

    /**
     * Get a connection to a Redis server
     * @param uri The URI of the Redis server
     * @return The connection to the Redis server
     * @throws std::runtime_error if the connection fails
     */
    std::shared_ptr<sw::redis::Redis> getConnection(const std::string &uri);

private:
    std::unordered_map<std::string, std::shared_ptr<sw::redis::Redis>> connections;
    std::mutex mutex;
};
