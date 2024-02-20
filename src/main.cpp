#include "main.h"
#include "models/Customer.h"
#include "models/Supplier.h"

int main() {
    // Connect to PostgreSQL
    std::unique_ptr<pqxx::connection> postgresConn = initDatabase();
    if (!postgresConn) return EXIT_FAILURE;

    // Connect to Redis
    std::shared_ptr<sw::redis::Redis> redisContext = conn2Redis();
    if (!redisContext) return EXIT_FAILURE;

    // Program logic...
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Postgres and Redis are running.");

    // Testing
    /*    {
        Customer customer("Gigio", 100);
        customer.searchProduct(std::nullopt, std::nullopt, std::nullopt, std::nullopt, "amount", true);
        customer.getBalance();
        customer.setBalance(180, true);
        customer.getBalance();
        customer.setBalance(150, false);
        customer.getBalance();

        Supplier supplier("Pallo", 147);
        supplier.getBalance();
        supplier.setBalance(100, true);
        supplier.getBalance();
    }*/

    // Input loop here...

    // Terminating the program
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Closing connections...");

    return EXIT_SUCCESS;
}
