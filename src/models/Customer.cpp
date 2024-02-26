#include "Customer.h"

std::string Customer::getUserType() const { return "customer"; }

std::string Customer::toString() const {
    std::ostringstream oss;
    oss << "Customer: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}

void Customer::getBalance() const { User::getBalance(); }

void Customer::setBalance(const int32_t &balanceChange) { User::setBalance(balanceChange); }

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
        pqxx::work W(*conn);
        pqxx::result R = W.exec(query);
        W.commit();

        // Print results
        printRows(R);
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to search products: ", e.what());
    }
}
