#ifndef ECOMMERCE_SUPPLIER_H
#define ECOMMERCE_SUPPLIER_H

#include "model_utils.h"

/**
 * Implementation of a Supplier class.
 *
 * @details This class represents a supplier on the e-commerce platform.
 * A supplier offers products that can be bought by customers and delivered by transporters.
 */
class Supplier {
private:
    uint32_t supplierId; ///< Unique identifier of the supplier.
    std::string name; ///< Name of the supplier.
    uint32_t balance; ///< Balance of the supplier.
public:
    Supplier(const uint32_t &id, std::string name) : supplierId(id), name(std::move(name)) {
        // Initialize other attributes...
        balance = 0;
    }

    // Supplier methods...
};


#endif //ECOMMERCE_SUPPLIER_H
