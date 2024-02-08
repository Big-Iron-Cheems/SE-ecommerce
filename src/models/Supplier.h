#pragma once

#include "User.h"

/**
 * Implementation of a Supplier class.
 *
 * @details This class represents a supplier on the e-commerce platform.
 * A supplier offers products that can be bought by customers and delivered by transporters.
 */
class Supplier : public User {
public:
    Supplier(std::string name, const uint32_t &balance = 0) : User(std::move(name), balance) {
        // Initialize other attributes...
    }

    /**
     * @return a string representation of the Supplier.
     */
    [[nodiscard]] std::string toString() const override;

    // Supplier methods...

    // Balance related methods

    void getBalance() const override;

    void setBalance(const uint32_t &balanceChange, bool add) override;

    // Product related methods

    /**
     * Retrive all products sold by the supplier.
     */
    void getProducts() const;

    /**
     * Add a product to the supplier's catalog.
     * @param name the name of the product.
     * @param price the price of the product.
     * @param amount the stock of the product.
     * @param description the description of the product.
     */
    void addProduct(const std::string &name, const uint32_t &price, const uint32_t &amount, const std::string &description);

    /**
     * Remove a product from the supplier's catalog.
     * @param productId the id of the product to remove.
     */
    void removeProduct(const uint32_t &productId);

    /**
     * Edit a product from the supplier's catalog.
     * @param productId the id of the product to edit.
     * @param name the new name of the product.
     * @param price the new price of the product.
     * @param amount the new stock of the product.
     * @param description the new description of the product.
     */
    void editProduct(const uint32_t &productId,
                     const std::optional<std::string> &name,
                     const std::optional<uint32_t> &price,
                     const std::optional<uint32_t> &amount,
                     const std::optional<std::string> &description);

    // Order related methods

    /**
    * Get the history of orders.
    */
    void getOrdersHistory() const;

    /**
     * Get the status of an order.
     * @param orderId the id of the order to get the status of.
     */
    void getOrderStatus(const uint32_t &orderId) const;
};
