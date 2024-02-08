#include "Transporter.h"

std::string Transporter::toString() const {
    std::ostringstream oss;
    oss << "Transporter: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}

void Transporter::getBalance() const {
    // connect to `ecommerce` db as `transporter` user
    PGconn *conn = conn2Postgres("ecommerce", "transporter", "transporter");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'transporter': ", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // build the query
    std::string query = "SELECT balance FROM transporters WHERE id = " + std::to_string(id) + ";";

    // execute the query
    PGresult *res = execCommand(conn, query);
    if (!res) {
        PQfinish(conn);
        return;
    }

    // print the result
    Utils::log(Utils::LogLevel::TRACE, std::cout, "Balance: ", PQgetvalue(res, 0, 0));
}

void Transporter::setBalance(const uint32_t &balanceChange, bool add) {
    // Connect to the `ecommerce` database as the `transporter` user
    PGconn *conn = conn2Postgres("ecommerce", "transporter", "transporter");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'transporter': ", PQerrorMessage(conn));
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
