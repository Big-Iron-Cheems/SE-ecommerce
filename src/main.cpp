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
            customer->setBalance(dist(gen) * 100); // Set random positive balance
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

/**
 * Handle command line arguments.
 * @param argc the number of arguments
 * @param argv the arguments
 */
void handleArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            Utils::log(Utils::LogLevel::TRACE, std::cout,
                       "Usage: " + std::string(argv[0]) +
                               " [options]\n"
                               "Options:\n"
                               "\t-h, --help    Show this help message and exit\n"
                               "\t-v            Enable verbose logging to console\n");
            exit(EXIT_SUCCESS);
        } else if (arg == "-v") {
            Utils::logToConsole = true;
        } else {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "Unknown argument: " + arg);
            Utils::log(Utils::LogLevel::TRACE, std::cout,
                       "Usage: " + std::string(argv[0]) +
                               " [options]\n"
                               "Options:\n"
                               "\t-h, --help    Show this help message and exit\n"
                               "\t-v            Enable verbose logging to console\n");

            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    handleArgs(argc, argv);

    // Initialize the database
    initDatabase();
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Ready to work...");

    // Testing
    testUsersInteractions();

    // Terminating the program
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Exiting program...");
    return EXIT_SUCCESS;
}
