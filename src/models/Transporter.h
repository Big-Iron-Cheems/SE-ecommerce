#pragma once

#include "User.h"

/**
 * Implementation of a Transporter class.
 *
 * @details This class represents a transporter on the e-commerce platform.
 * A transporter delivers products offered by suppliers and bought by customers.
 */
class Transporter : public User {
protected:
    [[nodiscard]] std::string getUserType() const override;

public:
    Transporter(std::string name, const uint32_t &balance = 0) : User(std::move(name), balance) {
        // Initialize other attributes...
    }

    /**
     * @return a string representation of the Transporter.
     */
    [[nodiscard]] std::string toString() const override;

    // Transporter methods...

    // Balance related methods

    void getBalance() const override;

    void setBalance(const uint32_t &balanceChange, bool add) override;

    // Product related methods

    // Order related methods

    /**
    * Get the history of orders.
    */
    void getOrdersHistory() const;

    /**
     * From the orders to deliver, get the customer's name and the address to deliver the order.
     */
    void getOngoingOrdersInfo() const;

    /**
     * Set the status of an order.
     * @param orderId id of the order to update.
     * @param status status to set. // TODO: define the possible statuses as an enum.
     */
    void setOrderStatus(const uint32_t &orderId, const std::string &status);
};
