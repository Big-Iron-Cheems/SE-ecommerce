#include "rdutils.h"

std::unique_ptr<sw::redis::Redis> conn2Redis() {
    try {
        auto redis = std::make_unique<sw::redis::Redis>("tcp://127.0.0.1:6379");
        return redis;
    } catch (const sw::redis::Error &err) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to connect to Redis: {}", err.what()));
        return nullptr;
    }
}
