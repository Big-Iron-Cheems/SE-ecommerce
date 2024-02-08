#include "Supplier.h"

std::string Supplier::toString() const {
    std::ostringstream oss;
    oss << "Supplier: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}

void Supplier::getBalance() const {
    // connect to `ecommerce` db as `supplier` user
    PGconn *conn = conn2Postgres("ecommerce", "supplier", "supplier");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'supplier': ", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // build the query
    std::string query = "SELECT balance FROM suppliers WHERE id = " + std::to_string(id) + ";";

    // execute the query
    PGresult *res = execCommand(conn, query);
    if (!res) {
        PQfinish(conn);
        return;
    }

    // print the result
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Balance: ", PQgetvalue(res, 0, 0));
}

void Supplier::setBalance(const uint32_t &balanceChange, bool add) {
    // Connect to the `ecommerce` database as the `supplier` user
    PGconn *conn = conn2Postgres("ecommerce", "supplier", "supplier");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'supplier': ", PQerrorMessage(conn));
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
