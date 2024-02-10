#pragma once

#include "db/dbutils.h"
#include "redis/rdutils.h"

/**
 * Function to clean up resources
 * @param conn PostgreSQL connection
 * @param context Redis connection
 */
void cleanup(redisContext *context);
