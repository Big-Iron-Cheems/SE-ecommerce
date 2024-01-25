#ifndef ECOMMERCE_SUPPLIER_H
#define ECOMMERCE_SUPPLIER_H

#include "User.h"

/**
 * Implementation of a Supplier class.
 *
 * @details This class represents a supplier on the e-commerce platform.
 * A supplier offers products that can be bought by customers and delivered by transporters.
 */
class Supplier : public User {
public:
    Supplier(const uint32_t &id, std::string name, const uint32_t &balance = 0) : User(id, std::move(name), balance) {
        // Initialize other attributes...
    }

    /**
     * @return a string representation of the Supplier.
     */
    [[nodiscard]] std::string toString() const override;

    // Supplier methods...
};


#endif //ECOMMERCE_SUPPLIER_H
