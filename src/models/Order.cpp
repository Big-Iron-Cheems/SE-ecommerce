#include "Order.h"

std::string Order::orderStatusToString(Order::Status orderStatus) {
    switch (orderStatus) {
        case Order::Status::SHIPPED: return "shipped";
        case Order::Status::DELIVERED: return "delivered";
        case Order::Status::CANCELLED: return "cancelled";
        default: throw std::invalid_argument("Invalid order status");
    }
}
