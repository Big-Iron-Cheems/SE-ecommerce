#include "Customer.h"

User::UserType Customer::getUserType() const { return User::UserType::CUSTOMER; }

std::string Customer::toString() const {
    std::ostringstream oss;
    oss << "Customer: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}

void Customer::searchProduct(const std::optional<std::string> &name,
                             const std::optional<std::string> &supplierUsername,
                             const std::optional<uint32_t> &priceLowerBound,
                             const std::optional<uint32_t> &priceUpperBound,
                             const std::optional<std::string> &orderBy,
                             const std::optional<bool> &sortDescending) const {
    try {
        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        std::unique_ptr<pqxx::connection> conn = conn2Postgres("ecommerce", "customer", "customer");

        // Build query from parameters
        std::string query = "SELECT * FROM products";
        std::string filters;

        // Filters
        if (name.has_value()) filters += "name LIKE " + conn->quote("%" + name.value() + "%") + " AND ";
        if (supplierUsername.has_value()) filters += "supplier_username LIKE " + conn->quote("%" + supplierUsername.value() + "%") + " AND ";
        if (priceLowerBound.has_value()) filters += "price >= " + std::to_string(priceLowerBound.value()) + " AND ";
        if (priceUpperBound.has_value()) filters += "price <= " + std::to_string(priceUpperBound.value()) + " AND ";

        // Remove the last " AND " if it exists
        if (!filters.empty() && filters.substr(filters.length() - 5) == " AND ") filters = filters.substr(0, filters.length() - 5);

        // Add filters to the query if they exist
        if (!filters.empty()) query += " WHERE " + filters;

        // Sort
        if (orderBy.has_value()) {
            query += " ORDER BY " + orderBy.value();
            if (sortDescending.value_or(false)) query += " DESC";
        }
        query += ";";

        // Execute query
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        // Print results
        printRows(R);
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to search products: ", e.what());
    }
}

void Customer::addProductToCart(const uint32_t &productId, const std::optional<uint32_t> &amount) {
    /*
     * It is intended to call this after a product has been found with `searchProduct`.
     * 1. Look for the given product id in the `products` table.
     *  1.1. If the product is not found, log an error and return.
     *  1.2. If the product is found, continue.
     * 2. Add X amount of the product to the cart.
     */

    // Validate amount
    if (amount.has_value() && amount.value() <= 0) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to add product to cart, invalid amount: ", amount.value());
        return;
    } else if (!amount.has_value()) Utils::log(Utils::LogLevel::TRACE, std::cout, "Quantity not provided, defaulting to 1.");


    try {
        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        std::unique_ptr<pqxx::connection> pgConn = conn2Postgres("ecommerce", "customer", "customer");

        // Build query from parameters
        std::string query = "SELECT name, supplier_id, price FROM products WHERE id = " + std::to_string(productId) + ";";

        // Execute query
        pqxx::work tx(*pgConn);
        auto [name, supplierId, price] = tx.query1<std::string, std::string, std::string>(query);
        tx.commit();

        // Add product to cart
        std::shared_ptr<sw::redis::Redis> rdConn = conn2Redis();

        std::string cartKey = "cart:" + id;
        std::string productKey = cartKey + ":" + std::to_string(productId);

        std::unordered_map<std::string, std::string> productData{{"name", name}, {"supplierId", supplierId}, {"price", price}, {"amount", std::to_string(amount.value())}};
        rdConn->hset(productKey, productData.begin(), productData.end());

        Utils::log(Utils::LogLevel::TRACE, std::cout, "Added ", amount.value(), "x `", name, "` to the cart.");
        // TODO: if successful, update the cart's total price?

    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to add product to cart: ", e.what());
    }

    try {
        std::shared_ptr<sw::redis::Redis> conn = conn2Redis();
    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to add product to cart: ", e.what());
    }
}

void Customer::removeProductFromCart(const uint32_t &productId, const std::optional<uint32_t> &amount) {
    /*
     * It is intended to call this after the cart has been listed with `getCart`.
     *
     * 1. Look for the given product id in the cart.
     *  1.1. If the product is not found, log an error and return.
     *  1.2. If the product is found but the amount is not enough, log an error and return. (Or remove the product from the cart?)
     *  1.3. If the product is found and the amount is enough, continue.
     * 2. Remove X amount of the product from the cart.
     */

    // Validate amount
    if (amount.has_value() && amount.value() <= 0) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to remove product from cart, invalid amount: ", amount.value());
        return;
    } else if (!amount.has_value()) Utils::log(Utils::LogLevel::TRACE, std::cout, "Quantity not provided, defaulting to max.");

    try {
        // Connect to the redis server
        std::shared_ptr<sw::redis::Redis> conn = conn2Redis();

        // Get the product from cart
        std::string cartKey = "cart:" + id;
        std::string productKey = cartKey + ":" + std::to_string(productId);

        // Get the product's current amount to decide if and how much to remove
        std::optional<std::string> currentQuantity = conn->hget(productKey, "amount");
        if (!currentQuantity.has_value()) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to remove product from cart, product not found in cart.");
            return;
        } else if (amount.has_value() && std::stoi(currentQuantity.value()) < amount.value()) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to remove product from cart, not enough amount in cart.");
            return;
        }

        // Remove the product from the cart
        if (amount.has_value() && std::stoi(currentQuantity.value()) > amount.value()) {
            conn->hset(productKey, "amount", std::to_string(std::stoi(currentQuantity.value()) - amount.value()));
        } else {
            conn->del(productKey);
        }

    } catch (sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to remove product from cart: ", e.what());
    }
}

void Customer::getCart() const {
    try {
        // Connect to the redis server
        std::shared_ptr<sw::redis::Redis> conn = conn2Redis();

        // Retrieve the keys of the products in the cart of the customer
        std::string cartKey = "cart:" + id;
        std::unordered_set<std::string> keys;
        long long cursor = 0LL;
        while (true) {
            cursor = conn->scan(cursor, cartKey + "*", 10, std::inserter(keys, keys.begin()));
            if (cursor == 0) break;
        }

        // Iterate over the keys and print the contents of the hashsets
        std::unordered_map<std::string, std::string> hash_data;
        std::ostringstream oss;
        for (const auto &key: keys) {
            conn->hgetall(key, std::inserter(hash_data, hash_data.begin()));

            oss << "\nProduct: " << key << " {\n";
            for (const auto &field: hash_data) {
                oss << "\t" << field.first << ": " << field.second << std::endl;
            }
            oss << "}";
        }
        Utils::log(Utils::LogLevel::TRACE, std::cout, oss.str());

    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to get cart: ", e.what());
    }
}
