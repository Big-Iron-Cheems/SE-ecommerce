#include "rdutils.h"

std::shared_ptr<sw::redis::Redis> conn2Redis() { return RedisConnectionPool::getInstance().getConnection("tcp://127.0.0.1:6379"); }
