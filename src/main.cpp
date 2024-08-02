#include "db/dbutils.h"
#include "models/Customer.h"
#include "models/Supplier.h"
#include "models/Transporter.h"

void testUsersInteractions() {
    Utils::log(Utils::LogLevel::DEBUG, std::cout, "Testing users interactions...");

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    std::uniform_int_distribution<> priceDist(1, 50);
    std::uniform_int_distribution<> amountDist(1, 10);

    // Create users
    std::vector<std::unique_ptr<User>> users;
    for (int i = 0; i < 3; ++i) {
        users.push_back(std::make_unique<Supplier>("Supplier" + std::to_string(i + 1)));
        users.push_back(std::make_unique<Customer>("Customer" + std::to_string(i + 1)));
        users.push_back(std::make_unique<Transporter>("Transporter" + std::to_string(i + 1)));
    }

    for (auto &user: users) {
        if (auto supplier = dynamic_cast<Supplier *>(user.get())) {
            // Add products to suppliers
            for (int j = 0; j < 3; ++j) {
                supplier->addProduct("Product" + std::to_string(j + 1), priceDist(gen), amountDist(gen), "Description" + std::to_string(j + 1));
            }
        } else if (auto customer = dynamic_cast<Customer *>(user.get())) {
            // Set random balance and add products to cart
            customer->setBalance(dist(gen) * 10); // Set random positive balance
            for (int j = 0; j < 3; ++j) {
                customer->addProductToCart(j + 1, amountDist(gen)); // Add random amount of products to cart
            }
            customer->makeOrder("Random Address " + std::to_string(dist(gen)));
        } else if (auto transporter = dynamic_cast<Transporter *>(user.get())) {
            // Get orders history for transporters
            transporter->getOrdersHistory();
        }
    }
}

int main() {
    // Initialize the database
    initDatabase();
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Ready to work...");

    // Testing
    testUsersInteractions();

    // Terminating the program
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Exiting program...");
    return EXIT_SUCCESS;
}
