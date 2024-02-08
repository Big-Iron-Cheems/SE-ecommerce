#pragma once

#include "../utils.h"
#include <hiredis/hiredis.h>

/**
 * Connect to Redis
 * @return a pointer to the connection object
 */
redisContext *conn2Redis();
