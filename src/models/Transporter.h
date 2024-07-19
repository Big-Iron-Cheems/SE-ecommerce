#pragma once

#include "Order.h"
#include "User.h"

/**
 * Implementation of a Transporter class.
 *
 * @details This class represents a transporter on the e-commerce platform.
 * A transporter delivers products offered by suppliers and bought by customers.
 */
class Transporter : public User {
protected:
    [[nodiscard]] UserType getUserType() const override;

public:
    explicit Transporter(std::string name) : User(std::move(name)) {
        try {
            User::openLogFile();
            User::login();
            loggedInSuccessfully = true;
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to create a Transporter: {}", e.what()));
        }
    }

    ~Transporter() override {
        if (loggedInSuccessfully) User::logout();
    };

    /**
     * @return a string representation of the Transporter.
     */
    [[nodiscard]] std::string toString() const override;

    // Transporter methods...

    // Product related methods

    // Order related methods

    /**
    * Get the history of orders.
    */
    [[maybe_unused]] void getOrdersHistory() const;

    /**
     * From the orders to deliver, get the customer's name and the address to deliver the order.
     */
    [[maybe_unused]] void getOngoingOrdersInfo() const;

    /**
     * Set the status of an order.
     * @param orderId id of the order to update.
     * @param orderStatus status to set.
     */
    [[maybe_unused]] void setOrderStatus(const uint32_t &orderId, Order::Status orderStatus);
};
