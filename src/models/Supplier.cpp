#include "Supplier.h"

User::UserType Supplier::getUserType() const { return User::UserType::SUPPLIER; }

std::string Supplier::toString() const {
    std::ostringstream oss;
    oss << "Supplier: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << getBalance() << "\n"
        << "}";
    return oss.str();
}

void Supplier::getProducts(const std::optional<std::string> &name,
                           const std::optional<uint32_t> &priceLowerBound,
                           const std::optional<uint32_t> &priceUpperBound,
                           const std::optional<std::vector<std::pair<std::string, bool>>> &orderBy) const {
    try {
        // Connect to `ecommerce` db as `supplier`
        auto conn = conn2Postgres("ecommerce", "supplier", "supplier");

        // Build query from parameters
        std::string query = std::format("SELECT * FROM products WHERE supplier_id = {}", id);
        std::string filters;

        // Filters
        if (name) filters += std::format("name LIKE '%{}%' AND ", name.value());
        if (priceLowerBound) filters += std::format("price >= {} AND ", priceLowerBound.value());
        if (priceUpperBound) filters += std::format("price <= {} AND ", priceUpperBound.value());

        // Remove the last " AND " if it exists
        if (!filters.empty() && filters.ends_with(" AND ")) filters = filters.substr(0, filters.length() - 5);

        // Add filters to the query if they exist
        if (!filters.empty()) query += " AND " + filters;

        // Sort
        if (orderBy) {
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
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to search products: {}", e.what()));
    }
}

void Supplier::addProduct(const std::string &name, const uint32_t &price, const uint32_t &amount, const std::string &description) {
    try {
        // Connect to `ecommerce` db as `supplier`
        auto conn = conn2Postgres("ecommerce", "supplier", "supplier");

        // Build the query to call the stored procedure
        std::string query = std::format("SELECT add_product('{}', {}, {}, {}, '{}');", name, id, price, amount, description);

        // Execute query
        pqxx::work tx(*conn);
        auto newProductId = tx.query_value<uint32_t>(query);
        tx.commit();

        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Product added successfully: {}", newProductId));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to add a product: {}", e.what()));
    }
}

void Supplier::removeProduct(const uint32_t &productId) {
    try {
        // Connect to `ecommerce` db as `supplier`
        auto conn = conn2Postgres("ecommerce", "supplier", "supplier");

        // Build the query to call the stored procedure
        std::string query = std::format("SELECT remove_product({});", productId);

        // Execute query
        pqxx::work tx(*conn);
        auto removedProductId = tx.query_value<uint32_t>(query);
        tx.commit();

        // if removedProductId is 0 then the product was not removed, log accordingly
        if (!removedProductId) Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to remove a product: product with id {} does not exist", productId));
        else Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Product removed successfully: {}", removedProductId));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to remove a product: {}", e.what()));
    }
}

void Supplier::editProduct(const uint32_t &productId,
                           const std::optional<std::string> &name,
                           const std::optional<uint32_t> &price,
                           const std::optional<uint32_t> &amount,
                           const std::optional<std::string> &description) {
    try {
        // Connect to `ecommerce` db as `supplier`
        auto conn = conn2Postgres("ecommerce", "supplier", "supplier");

        // Build the query to call the stored procedure
        std::string query = std::format("SELECT edit_product({}, {}, {}, {}, {});", productId,
                                        name ? std::format("'{}'", name.value()) : "NULL",
                                        price ? std::format("{}", price.value()) : "NULL",
                                        amount ? std::format("{}", amount.value()) : "NULL",
                                        description ? std::format("'{}'", description.value()) : "NULL");

        // Execute query
        pqxx::work tx(*conn);
        auto editedProductId = tx.query_value<uint32_t>(query);
        tx.commit();

        // if editedProductId is 0 then the product was not edited, log accordingly
        if (!editedProductId) Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to edit a product: product with id {} does not exist", productId));
        else Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Product edited successfully: {}", editedProductId));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to edit a product: {}", e.what()));
    }
}

void Supplier::getOrdersHistory() const {
    try {
        // Connect to `ecommerce` db as `supplier` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "supplier", "supplier");

        // Check if the order exists
        std::string query = std::format("SELECT DISTINCT o.* FROM orders o JOIN order_items oi ON o.id = oi.order_id WHERE oi.supplier_id = {};", id);

        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::TRACE, *logFile, "No order history.");
            return;
        }
        printRows(R);
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to fetch order history: {}", e.what()));
    }
}

void Supplier::getOrderStatus(const uint32_t &orderId) const {
    try {
        // Connect to `ecommerce` db as `supplier` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "supplier", "supplier");

        // Check if the order exists
        std::string query = std::format("SELECT DISTINCT o.id, o.status FROM orders o JOIN order_items oi ON o.id = oi.order_id WHERE oi.supplier_id = {} AND o.id = {};", id, orderId);

        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::ERROR, *logFile, "Failed to get order status, order not found.");
            return;
        }

        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Order {} status: {}", orderId, R[0]["status"].c_str()));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to fetch order status: {}", e.what()));
    }
}
