#pragma once

#include "../Utils.h"
#include <sw/redis++/redis++.h>

/**
 * Connect to Redis
 * @return a pointer to the Redis connection object
 */
std::unique_ptr<sw::redis::Redis> conn2Redis();
