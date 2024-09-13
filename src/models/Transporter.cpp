#include "Transporter.h"

User::UserType Transporter::getUserType() const { return User::UserType::TRANSPORTER; }

std::string Transporter::toString() const {
    std::ostringstream oss;
    oss << "Transporter: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << getBalance() << "\n"
        << "}";
    return oss.str();
}

// TODO: pass the printRows result into the Utils::log function so that it can be logged in the file as well
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
            Utils::log(Utils::LogLevel::TRACE, *logFile, "No order history.");
            return;
        }
        printRows(R);
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to fetch order history: {}", e.what()));
    }
}

void Transporter::getOngoingOrdersInfo() const {
    try {
        // Connect to `ecommerce` db as `transporter` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "transporter", "transporter");

        // Check if the order exists
        std::string query = std::format("SELECT (get_ongoing_orders({})).*;", id);
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();

        if (R.empty()) {
            Utils::log(Utils::LogLevel::TRACE, *logFile, "No ongoing orders.");
            return;
        }
        printRows(R);
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to fetch ongoing orders: {}", e.what()));
    }
}

void Transporter::setOrderStatus(const uint32_t &orderId, Order::Status orderStatus) {
    try {
        // Connect to `ecommerce` db as `transporter` user using conn2Postgres
        auto conn = conn2Postgres("ecommerce", "transporter", "transporter");

        // Edit the status of the order
        std::string userType = userTypeToString(getUserType());
        std::string query = std::format("SELECT set_order_status('{}', {}, {}, '{}');", userType, id, orderId, Order::orderStatusToString(orderStatus));

        pqxx::work tx(*conn);
        tx.exec(query);
        tx.commit();

        Utils::log(Utils::LogLevel::TRACE, *logFile, std::format("Order {} status updated to {}.", orderId, Order::orderStatusToString(orderStatus)));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, *logFile, std::format("Failed to update order status: {}", e.what()));
    }
}
