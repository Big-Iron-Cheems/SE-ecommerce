#include "db/dbutils.h"
#include "models/Customer.h"
#include "models/Supplier.h"
#include "models/Transporter.h"

int main() {
    // Initialize the database
    initDatabase();
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Ready to work...");

    // Testing
    {
        // Customer customer("Gigio");
        // Customer customer2("Melo");
        // customer2.addProductToCart(8, 2);
        // customer.addProductToCart(6, 3);
        // customer.addProductToCart(5,1);
        // customer.makeOrder("Piazza Bibo 31");
        // customer.getCart();
        // customer.searchProduct("mele", std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        // customer.setBalance(100);
        // Customer customer = Customer("Gigio");
        // Supplier supplier("Piero");
        // Transporter transporter("Pippo");
        // customer.setBalance(-1);
        // customer.getBalance();
        // Customer customer2("Marco");
        // customer.searchProduct(std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::vector<std::pair<std::string, bool>>{{"amount", false}, {"price", false}});
        // customer.addProductToCart(1, 3);
        // customer.addProductToCart(4, 2);
        // customer.addProductToCart(1, 47);

        // customer.getCart();
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

        // Supplier supplier("Piero");
        // supplier.getProducts("mele", std::nullopt, std::nullopt, std::nullopt);
        // supplier.addProduct("arancia", 3, 14, "arancia di sicilia");
        // supplier.getProducts("arancia", std::nullopt, std::nullopt, std::nullopt);
        // supplier.removeProduct(7);
        // supplier.removeProduct(17);
        // supplier.editProduct(5, "Cipolle", 2, 41, "Cipolle di tropea");
        // supplier.getOrdersHistory();
        // supplier.getOrderStatus(1);

        // Transporter transporter("Pippo");
        // transporter.getOrdersHistory();
        // transporter.getOngoingOrdersInfo();
        // transporter.setOrderStatus(1, Order::Status::DELIVERED);
        // transporter.setOrderStatus(2, Order::Status::DELIVERED); // Error
    }

    // Terminating the program
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Exiting program...");
    return EXIT_SUCCESS;
}
