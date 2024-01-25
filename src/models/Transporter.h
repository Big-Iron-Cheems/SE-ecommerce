#ifndef ECOMMERCE_TRANSPORTER_H
#define ECOMMERCE_TRANSPORTER_H

#include "User.h"

/**
 * Implementation of a Transporter class.
 *
 * @details This class represents a transporter on the e-commerce platform.
 * A transporter delivers products offered by suppliers and bought by customers.
 */
class Transporter : public User {
public:
    Transporter(const uint32_t &id, std::string name, const uint32_t &balance = 0) : User(id, std::move(name), balance) {
        // Initialize other attributes...
    }

    /**
     * @return a string representation of the Transporter.
     */
    [[nodiscard]] std::string toString() const override;

    // Transporter methods...
};


#endif //ECOMMERCE_TRANSPORTER_H
