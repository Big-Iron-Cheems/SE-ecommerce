#ifndef ECOMMERCE_CUSTOMER_H
#define ECOMMERCE_CUSTOMER_H

#include "User.h"

/**
 * Implementation of a Customer class.
 *
 * @details This class represents a customer on the e-commerce platform.
 * A customer buys products offered by suppliers and delivered by transporters.
 * Customers can also return products.
 */
class Customer : public User {
public:
    Customer(const uint32_t &id, std::string name, const uint32_t &balance = 0) : User(id, std::move(name), balance) {
        // Initialize other attributes...
    }

    /**
     * @return a string representation of the Customer.
     */
    [[nodiscard]] std::string toString() const override;


    // Customer methods...
    /*
     * A Customer can:
     * - buy a product from a Supplier
     * - return a product to a Supplier
     * - receive a product from a Transporter
     * - cancel an order
     * - search for a product
     *
     */
};


#endif //ECOMMERCE_CUSTOMER_H
