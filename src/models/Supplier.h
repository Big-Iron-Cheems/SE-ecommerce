#pragma once

#include "User.h"

/**
 * Implementation of a Supplier class.
 *
 * @details This class represents a supplier on the e-commerce platform.
 * A supplier offers products that can be bought by customers and delivered by transporters.
 */
class Supplier : public User {
protected:
    [[nodiscard]] UserType getUserType() const override;

public:
    explicit Supplier(std::string name) : User(std::move(name)) {
        try {
            User::openLogFile();
            User::login();
            loggedInSuccessfully = true;
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to create a Supplier: {}", e.what()));
        }
    }

    ~Supplier() override {
        if (loggedInSuccessfully) User::logout();
    };

    /**
     * @return a string representation of the Supplier.
     */
    [[nodiscard]] std::string toString() const override;

    // Supplier methods...
    // Product related methods

    /**
     * Retrive all products sold by the supplier.
     */
    void getProducts(const std::optional<std::string> &name,
                                      const std::optional<uint32_t> &priceLowerBound,
                                      const std::optional<uint32_t> &priceUpperBound,
                                      const std::optional<std::vector<std::pair<std::string, bool>>> &orderBy) const;

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
