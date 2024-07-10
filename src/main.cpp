#include "main.h"
#include "models/Customer.h"
#include "models/Supplier.h"
#include "models/Transporter.h"

int main() {
    // Connect to PostgreSQL
    std::unique_ptr<pqxx::connection> postgresConn = initDatabase();
    if (!postgresConn) return EXIT_FAILURE;

    // Connect to Redis
    std::shared_ptr<sw::redis::Redis> redisConn = conn2Redis();
    if (!redisConn) return EXIT_FAILURE;

    // Program logic...
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Postgres and Redis are running.");

    // Testing
    {
        Customer customer("Gigio");
        customer.setBalance(100);
        // Customer customer = Customer("Gigio");
        // Supplier supplier("Piero");
        // Transporter transporter("Pippo");
        // customer.setBalance(-1);
        // customer.getBalance();
        // Customer customer2("Marco");
        customer.searchProduct(std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::vector<std::pair<std::string, bool>>{{"amount", false}, {"price", false}});
        // customer.addProductToCart(1, 3);
        // customer.addProductToCart(4, 2);
        // customer.addProductToCart(1, 47);

        customer.getCart();
        // customer.makeOrder("Via delle Palme 69");
        // customer.clearCart();
        // customer.getCart();

        // customer2.getBalance();
        // customer2.setBalance(100);
        // customer2.getBalance();
        // customer2.setBalance(-70);
        // customer2.getBalance();

        // Customer customer2err("Gigio");
        // Customer customer3("Marco");
        // Customer customer4err("Marco");
        // Customer customer5("Pippo");

        // Supplier supplier("Pallo");
        // supplier.getBalance();
        // supplier.setBalance(100);
        // supplier.getBalance();
    }

    // Terminating the program
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Closing connections...");

    return EXIT_SUCCESS;
}
