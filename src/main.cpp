#include "main.h"

int main() {
    // Connect to PostgreSQL
    PGconn *postgresConn = initDatabase();
    if (!postgresConn) return EXIT_FAILURE;


    // Connect to Redis
    redisContext *redisContext = conn2Redis();
    if (!redisContext) {
        cleanup(postgresConn, nullptr);
        return EXIT_FAILURE;
    }

    // Program logic...
    tracePrint("Postgres and Redis are running.");

    // Terminating the program
    cleanup(postgresConn, redisContext);

    return EXIT_SUCCESS;
}

void cleanup(PGconn *conn, redisContext *context) {
    tracePrint("Closing connections...");
    if (conn) PQfinish(conn);
    if (context) redisFree(context);
}
