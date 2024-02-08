#include "Customer.h"

std::string Customer::toString() const {
    std::ostringstream oss;
    oss << "Customer: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}


void Customer::getBalance() const {
    // connect to `ecommerce` db as `customer` user
    PGconn *conn = conn2Postgres("ecommerce", "customer", "customer");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'customer': ", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // build the query
    std::string query = "SELECT balance FROM customers WHERE id = " + std::to_string(id) + ";";

    // execute the query
    PGresult *res = execCommand(conn, query);
    if (!res) {
        PQfinish(conn);
        return;
    }

    // print the result
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Balance: ", PQgetvalue(res, 0, 0));
}

void Customer::setBalance(const uint32_t &balanceChange, bool add) {
    // Connect to the `ecommerce` database as the `customer` user
    PGconn *conn = conn2Postgres("ecommerce", "customer", "customer");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'customer': ", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // Build the query to call the stored procedure
    std::string query = "SELECT set_balance_customer(" + std::to_string(id) + ", " + std::to_string(balanceChange) + ", " + (add ? "TRUE" : "FALSE") + ");";

    // Execute the query
    PGresult *res = execCommand(conn, query);
    if (!res) {
        PQfinish(conn);
        return;
    }

    // Print the result
    Utils::log(Utils::LogLevel::TRACE, std::cout, (add ? "Balance increased by " : "Balance decreased by "), PQgetvalue(res, 0, 0));

    PQclear(res);
    PQfinish(conn);
}

void Customer::searchProduct(const std::optional<std::string> &name,
                             const std::optional<std::string> &supplierUsername,
                             const std::optional<uint32_t> &priceLowerBound,
                             const std::optional<uint32_t> &priceUpperBound,
                             const std::optional<std::string> &orderBy,
                             const std::optional<bool> &sortDescending) const {
    // connect to `ecommerce` db as `customer` user
    PGconn *conn = conn2Postgres("ecommerce", "customer", "customer");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'customer': ", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // build query from parameters
    std::string query = "SELECT * FROM products";
    std::string filters;

    // Filters
    if (name.has_value()) filters += "name LIKE '%" + name.value() + "%' AND ";
    if (supplierUsername.has_value()) filters += "supplier_username LIKE '%" + supplierUsername.value() + "%' AND ";
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
    PGresult *res = execCommand(conn, query);
    int rows = PQntuples(res);
    if (rows == 0) {
        Utils::log(Utils::LogLevel::TRACE, std::cout, "No products found.");
        PQclear(res);
        PQfinish(conn);
        return;
    }

    // TODO: create function to print query results as a table (with headers)
    // print results
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Found ", rows, " products:");
    for (int i = 0; i < rows; i++) {
        Utils::log(Utils::LogLevel::TRACE, std::cout, "\tid ", i, ": ");
        int cols = PQnfields(res);
        for (int j = 0; j < cols; j++) { Utils::log(Utils::LogLevel::TRACE, std::cout, PQgetvalue(res, i, j), " | "); }
        std::cout << std::endl;
    }
    PQclear(res);
    PQfinish(conn);
}
