#ifndef ECOMMERCE_CUSTOMER_H
#define ECOMMERCE_CUSTOMER_H


#include "model_utils.h"

/**
 * Implementation of a Customer class.
 *
 * @details This class represents a customer on the e-commerce platform.
 * A customer buys products offered by suppliers and delivered by transporters.
 * Customers can also return products.
 */
class Customer {
private:
    uint32_t customerId; ///< Unique identifier of the customer.
    std::string name; ///< Name of the customer.
    uint32_t balance; ///< Balance of the customer.
public:
    Customer(const uint32_t &id, std::string name) : customerId(id), name(std::move(name)) {
        // Initialize other attributes...
        balance = 0;
    }

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
