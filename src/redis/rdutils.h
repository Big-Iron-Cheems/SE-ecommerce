#ifndef ECOMMERCE_RDUTILS_H
#define ECOMMERCE_RDUTILS_H

#include <hiredis/hiredis.h>
#include "../utils.h"

/**
 * Connect to Redis
 * @return a pointer to the connection object
 */
redisContext *conn2Redis();

#endif //ECOMMERCE_RDUTILS_H
