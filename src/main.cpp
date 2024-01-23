#include "main.h"

void cleanup(PGconn *conn, redisContext *context) {
    fprintf(stdout, "Closing connections...\n");
    if (conn) PQfinish(conn);
    if (context) redisFree(context);
}

int main() {
    // Connect to PostgreSQL
    PGconn *postgresConn = conn2Postgres();
    if (!postgresConn) {
        return EXIT_FAILURE;
    }

    // Connect to Redis
    redisContext *redisContext = conn2Redis();
    if (!redisContext) {
        cleanup(postgresConn, nullptr);
        return EXIT_FAILURE;
    }

    // Program logic...
    fprintf(stdout, "Postgres and Redis are running.\n");

    // Create a new PostgreSQL user
    createUser(postgresConn, "ecommerce", "ecommerce");

    // Init the tables
    initTables(postgresConn);

    // Terminating the program
    cleanup(postgresConn, redisContext);

    return EXIT_SUCCESS;
}
