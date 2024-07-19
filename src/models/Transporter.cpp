#include "Transporter.h"

User::UserType Transporter::getUserType() const { return User::UserType::TRANSPORTER; }

std::string Transporter::toString() const {
    std::ostringstream oss;
    oss << "Transporter: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}

void Transporter::getOrdersHistory() const {
    try {
        // Connect to `ecommerce` db as `transporter` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "transporter", "transporter");

        // Check if the order exists
        std::string query = std::format("SELECT * FROM orders WHERE transporter_id = {};", id);

        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::TRACE, std::cout, "No order history.");
            return;
        }
        printRows(R);
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to fetch order history: {}", e.what()));
    }
}

void Transporter::getOngoingOrdersInfo() const {
    try {
        // Connect to `ecommerce` db as `transporter` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "transporter", "transporter");

        // Check if the order exists
        std::string query = std::format("SELECT o.id, c.username, o.address FROM orders o JOIN customers c ON o.customer_id = c.id WHERE o.transporter_id = {} AND o.status = 'shipped';", id);

        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::TRACE, std::cout, "No ongoing orders.");
            return;
        }
        printRows(R);
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to fetch ongoing orders: {}", e.what()));
    }
}

void Transporter::setOrderStatus(const uint32_t &orderId, Order::Status orderStatus) {
    try {
        // Connect to `ecommerce` db as `transporter` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "transporter", "transporter");

        // Edit the status of the order
        std::string query = std::format("SELECT set_order_status({}, {}, '{}');", id, orderId, Order::orderStatusToString(orderStatus));

        pqxx::work tx(*conn);
        tx.exec(query);
        tx.commit();

        Utils::log(Utils::LogLevel::TRACE, std::cout, std::format("Order {} status updated to {}.", orderId, Order::orderStatusToString(orderStatus)));
    } catch (const pqxx::broken_connection &e) {
        throw; // Rethrow the exception to propagate it to the caller
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to update order status: {}", e.what()));
    }
}
