#include "rdutils.h"

redisContext *conn2Redis() {
    redisContext *context = redisConnect("localhost", 6379);
    if (context == nullptr || context->err) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Redis: ", context ? context->errstr : "Unknown error");
        if (context) redisFree(context);
        return nullptr;
    }

    return context;
}
