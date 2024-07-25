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

// TODO: replace the params with a structured object (struct or class)
void Customer::searchProduct(const std::optional<std::string> &name,
                             const std::optional<std::string> &supplierUsername,
                             const std::optional<uint32_t> &priceLowerBound,
                             const std::optional<uint32_t> &priceUpperBound,
                             const std::optional<std::vector<std::pair<std::string, bool>>> &orderBy) const {
    try {
        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "customer", "customer");

        // Build query from parameters
        std::string query = "SELECT * FROM products";
        std::string filters;

        // Filters
        if (name.has_value()) filters += std::format("name LIKE '%{}%' AND ", name.value());
        if (supplierUsername.has_value()) filters += std::format("supplier_username LIKE '%{}%' AND ", supplierUsername.value());
        if (priceLowerBound.has_value()) filters += std::format("price >= {} AND", priceLowerBound.value());
        if (priceUpperBound.has_value()) filters += std::format("price <= {} AND", priceUpperBound.value());

        // Remove the last " AND " if it exists
        if (!filters.empty() && filters.ends_with(" AND ")) filters = filters.substr(0, filters.length() - 5);

        // Add filters to the query if they exist
        if (!filters.empty()) query += " WHERE " + filters;

        // Sort
        if (orderBy.has_value()) {
            query += " ORDER BY ";
            for (const auto &[columnName, sortDescending]: orderBy.value()) {
                query += columnName;
                if (sortDescending) query += " DESC";
                query += ", ";
            }
            // Remove the last ", " if it exists
            if (!orderBy.value().empty()) query = query.substr(0, query.length() - 2);
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
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to search products: {}", e.what()));
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
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to add product to cart, invalid amount: {}", amount.value()));
        return;
    } else if (!amount.has_value()) Utils::log(Utils::LogLevel::TRACE, *logFile, "Quantity not provided, defaulting to 1.");


    try {
        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        auto pgConn = conn2Postgres("ecommerce", "customer", "customer");

        // Build query from parameters
        std::string query = std::format("SELECT name, supplier_id, price FROM products WHERE id = {};", productId);

        // Execute query
        pqxx::work tx(*pgConn);
        auto [name, supplierId, price] = tx.query1<std::string, std::string, std::string>(query);
        tx.commit();

        // Add product to cart
        auto rdConn = conn2Redis();

        std::string productKey = std::format("cart:{}:{}", id, productId);
        std::unordered_map<std::string, std::string> productData{{"name", name}, {"supplierId", supplierId}, {"price", price}, {"amount", std::to_string(amount.value_or(1))}};
        rdConn->hset(productKey, productData.begin(), productData.end());

        // Update the total price of the cart
        uint32_t productTotalPrice = std::stoi(price) * amount.value_or(1);
        std::string totalPriceKey = std::format("cart:{}:total_price", id);
        if (!rdConn->exists(totalPriceKey)) rdConn->set(totalPriceKey, "0");
        rdConn->incrby(totalPriceKey, productTotalPrice);

        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Added {}x `{}` to the cart. Total price updated by {}", amount.value_or(1), name, productTotalPrice));
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to add product to cart: {}", e.what()));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to add product to cart: {}", e.what()));
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
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to remove product from cart, invalid amount: {}", amount.value()));
        return;
    } else if (!amount.has_value()) Utils::log(Utils::LogLevel::TRACE, *logFile, "Quantity not provided, defaulting to max.");

    try {
        // Connect to the redis server
        auto conn = conn2Redis();

        // Get the product from cart
        std::string productKey = std::format("cart:{}:{}", id, productId);

        // Get the product's current amount to decide if and how much to remove
        std::optional<std::string> currentQuantity = conn->hget(productKey, "amount");
        std::optional<std::string> price = conn->hget(productKey, "price");
        if (!currentQuantity.has_value()) {
            Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to remove product from cart, product not found in cart.");
            return;
        } else if (amount.has_value() && std::stoi(currentQuantity.value()) < static_cast<int>(amount.value())) {
            Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to remove product from cart, not enough amount in cart.");
            return;
        }

        // Calculate the total price to be subtracted
        uint32_t productTotalPrice = std::stoi(price.value()) * amount.value_or(std::stoi(currentQuantity.value()));

        // Remove the product from the cart
        if (amount.has_value() && std::stoi(currentQuantity.value()) > static_cast<int>(amount.value())) {
            conn->hincrby(productKey, "amount", -static_cast<int>(amount.value()));
        } else {
            conn->del(productKey);
        }

        // Update the total price of the cart
        std::string totalPriceKey = std::format("cart:{}:total_price", id);
        conn->decrby(totalPriceKey, productTotalPrice);

        Utils::log(Utils::LogLevel::TRACE, *logFile,
                   std::format("Removed {}x product from the cart. Total price updated by {}", amount.value_or(std::stoi(currentQuantity.value())), -productTotalPrice));
    } catch (sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to remove product from cart: {}", e.what()));
    }
}

// TODO printCart method, uses getCart and prints the cart in a more user-friendly way
std::map<std::string, std::unordered_map<std::string, std::string>> Customer::getCart() const {
    std::map<std::string, std::unordered_map<std::string, std::string>> completeCart;
    try {
        // Connect to the redis server
        auto conn = conn2Redis();

        // Retrieve the prodKeys of the products in the cart of the customer
        std::string pattern = std::format("cart:{}:*", id);
        std::unordered_set<std::string> prodKeys;
        long long cursor = 0LL;
        do {
            cursor = conn->scan(cursor, pattern, 10, std::inserter(prodKeys, prodKeys.end()));
        } while (cursor != 0);
        prodKeys.erase(std::format("cart:{}:total_price", id));

        // Iterate over the prodKeys and print the contents of the hashsets
        uint32_t totalPrice = 0;
        std::ostringstream oss;
        for (const auto &key: prodKeys) {
            std::unordered_map<std::string, std::string> prodData;
            conn->hgetall(key, std::inserter(prodData, prodData.begin()));
            completeCart[key.substr(key.find(':', key.find(':') + 1) + 1)] = prodData;

            oss << "\nProduct: " << key << " {\n";
            for (const auto &[field, value]: prodData) {
                oss << "\t" << field << ": " << value << std::endl;
            }
            oss << "}";

            totalPrice += std::stoi(prodData["price"]) * std::stoi(prodData["amount"]);
        }
        oss << "\nTotal price: " << totalPrice << std::endl;
        Utils::log(Utils::LogLevel::TRACE, *logFile, oss.str());

    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to get cart: {}", e.what()));
    }

    return completeCart;
}

uint32_t Customer::getCartTotalPrice() const {
    try {
        // Connect to the redis server
        auto conn = conn2Redis();

        // Get the total price
        std::string totalPriceKey = std::format("cart:{}:total_price", id);
        std::optional<std::string> totalPrice = conn->get(totalPriceKey);

        return totalPrice.has_value() ? std::stoi(totalPrice.value()) : 0;
    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to get total price of cart: {}", e.what()));
        return 0;
    }
}

void Customer::clearCart() {
    try {
        // Connect to the redis server
        auto conn = conn2Redis();

        // Retrieve all keys for the customer's cart products
        std::string pattern = std::format("cart:{}:*", id);
        std::unordered_set<std::string> keysToDelete;
        auto cursor = 0LL;
        do {
            cursor = conn->scan(cursor, pattern, 10, std::inserter(keysToDelete, keysToDelete.end()));
        } while (cursor != 0);

        // Delete each key
        for (const auto &key: keysToDelete) {
            conn->del(key);
        }

        // Delete the main cart key
        std::string mainCartKey = "cart:" + id;
        conn->del(mainCartKey);

        Utils::log(Utils::LogLevel::TRACE, *logFile, "Cart cleared.");
    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to clear cart: {}", e.what()));
    }
}

void Customer::makeOrder(const std::string &address) {
    /*
     * It is intended to call this after the cart has been populated with `addProductToCart`.
     *
     * 1. Verify that all the items in the cart are still available. (Suppliers might have removed them from the platform.)
     *  1.1. If an iem is not available, log an error and return. (Maybe remove it silently from the cart? Counterintuitive.)
     *  1.2. If all items are available, continue.
     *  1.3. If the cart is empty, log an error and return.
     * 2. Create an order with the items in the cart.
     * 3. Remove the items from the Redis cart.
     * 4. Update the customer's and suppliers' balances.
     * 5. Update the products and orders tables.
     */

    try {
        // Verify that the user has enough balance
        uint32_t totalPrice = getCartTotalPrice();
        if (balance < totalPrice) {
            Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to make order, not enough balance.");
            return;
        }

        // Get the cart
        auto cart = getCart();
        if (cart.empty()) {
            Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to make order, cart is empty.");
            return;
        }

        // Connect to the redis server
        auto rdConn = conn2Redis();

        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "customer", "customer");
        pqxx::work tx(*conn);

        // Step 1: Insert the new order and get the order ID
        std::string insertOrderQuery = std::format("SELECT make_order({}, {}, '{}');", id, totalPrice, address);
        pqxx::result newOrderResult = tx.exec(insertOrderQuery);
        auto newOrderId = newOrderResult[0][0].as<uint32_t>();

        // Steps 2-4
        for (auto &[productKey, productData]: cart) {
            // Verify that each product is still available
            std::string query = std::format("SELECT * FROM products WHERE id = {};", productKey);
            pqxx::result R = tx.exec(query);

            if (R.empty()) {
                Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to make order, product not found in the database.");
                return;
            } else if (std::stoi(productData["amount"]) > R[0]["amount"].as<int32_t>()) {
                Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to make order, not enough stock for product {}", productKey));
                return;
            }

            // Step 2-3: Add each product to the order_items table, update the products table
            std::string insertOrderItemQuery = std::format("SELECT add_order_item({}, {}, {}, {}, {});", newOrderId, productKey, productData["amount"], productData["price"], productData["supplierId"]);
            Utils::log(Utils::LogLevel::ALERT, std::cout, insertOrderItemQuery);
            tx.exec(insertOrderItemQuery);

            // Step 4: Update the supplier's balance
            int32_t productPrice = std::stoi(productData["price"]) * std::stoi(productData["amount"]);
            std::string updateSupplierBalanceQuery = std::format("SELECT set_balance('supplier', {}, {});", productData["supplierId"], productPrice);
            Utils::log(Utils::LogLevel::ALERT, std::cout, updateSupplierBalanceQuery);
            tx.exec(updateSupplierBalanceQuery);
        }

        // Step 5: update the customer's balance
        setBalance(-static_cast<int32_t>(totalPrice));

        // Step 6: Remove items from Redis cart and reset the total price
        clearCart();

        // Commit the transaction
        tx.commit();
        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Order made: {}", newOrderId));
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const sw::redis::Error &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to make order: {}", e.what()));
    }
}

void Customer::cancelOrder(const uint32_t &orderId) {
    /*
     * It is intended to call this after the orders have been listed with `getOrdersHistory`.
     * This doesn't effectively delete the order rom the db, but it marks it as cancelled.
     */

    try {
        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "customer", "customer");

        // Check if the order exists
        std::string query = std::format("SELECT * FROM orders WHERE id = {};", orderId);
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to cancel order, order not found.");
            return;
        }
        // TODO: check the status enum to see if it can be actually cancelled

        // Cancel the order
        query = std::format("UPDATE orders SET status = 'cancelled' WHERE id = {};", orderId);
        tx.exec(query);
        tx.commit();

        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Order cancelled: {}", orderId));
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to cancel order: {}", e.what()));
    }
}

void Customer::getOrderStatus(const uint32_t &orderId) const {
    /*
     * It is intended to call this after the orders have been listed with `getOrdersHistory`.
     *
     * 1. Look for the given order id in the orders table.
     *  1.1. If the order is not found, log an error and return.
     *  1.2. If the order is found, continue.
     * 2. Print the status of the order.
     */

    try {
        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "customer", "customer");

        // Check if the order exists
        std::string query = std::format("SELECT status FROM orders WHERE id = {} AND customer_id = '{}';", orderId, id);
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to get order status, order not found.");
            return;
        }

        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Order {} status: {}", orderId, R[0]["status"].c_str()));
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to fetch order status: {}", e.what()));
    }
}

void Customer::getOrdersHistory() const {
    /*
     * 1. Look for the orders of the customer in the orders table.
     * 2. Print the orders of the customer.
     *
     * TODO Maybe allow optional filters and sorting?
     */

    try {
        // Connect to `ecommerce` db as `customer` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "customer", "customer");

        std::string query = std::format("SELECT * FROM orders WHERE customer_id = {};", id);
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::TRACE, *logFile, "No order history.");
            return;
        }
        printRows(R);
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to fetch order history: {}", e.what()));
    }
}
