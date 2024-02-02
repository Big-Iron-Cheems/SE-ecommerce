#include "main.h"
#include "models/Customer.h"

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

    // Testing
    /*{
        Customer customer(0, "Gigio", 100);
        customer.searchProduct(std::nullopt, std::nullopt, std::nullopt, std::nullopt, "amount", true);
        customer.getBalance();
        customer.setBalance(50, true);
        customer.getBalance();
        customer.setBalance(150, false);
        customer.getBalance();
    }*/

    // Terminating the program
    cleanup(postgresConn, redisContext);

    return EXIT_SUCCESS;
}

void cleanup(PGconn *conn, redisContext *context) {
    tracePrint("Closing connections...");
    if (conn) PQfinish(conn);
    if (context) redisFree(context);
}
