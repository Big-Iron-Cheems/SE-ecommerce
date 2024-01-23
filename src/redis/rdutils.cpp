#include "rdutils.h"

redisContext *conn2Redis() {
    redisContext *context = redisConnect("localhost", 6379);
    if (context == nullptr || context->err) {
        fprintf(stderr, "Failed to connect to Redis: %s.\n", context ? context->errstr : "Unknown error");
        if (context) redisFree(context);
        return nullptr;
    }

    return context;
}
