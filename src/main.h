#ifndef ECOMMERCE_MAIN_H
#define ECOMMERCE_MAIN_H

#include "db/dbutils.h"
#include "redis/rdutils.h"

/**
 * Function to clean up resources
 * @param conn PostgreSQL connection
 * @param context Redis connection
 */
void cleanup(PGconn *conn, redisContext *context);

#endif //ECOMMERCE_MAIN_H
