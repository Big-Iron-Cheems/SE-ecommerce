#pragma once

#include <stdexcept>
#include <string>

class Order {
public:
    Order() = delete;                              ///< Default constructor - deleted
    Order(const Order &other) = delete;            ///< Copy constructor - deleted
    Order(Order &&other) = delete;                 ///< Move constructor - deleted
    Order &operator=(const Order &other) = delete; ///< Copy assignment operator - deleted
    Order &operator=(Order &&other) = delete;      ///< Move assignment operator - deleted
    ~Order() = delete;                             ///< Destructor - deleted

    enum class Status {
        SHIPPED,   ///< Customer has paid for the order and it is shipped.
        DELIVERED, ///< The order has been delivered to the customer.
        CANCELLED  ///< The order has been cancelled by the customer.
    };

    static std::string orderStatusToString(Status orderStatus);
};
