#pragma once

#include "User.h"

/**
 * Implementation of a Customer class.
 *
 * @details This class represents a customer on the e-commerce platform.
 * A customer buys products offered by suppliers and delivered by transporters.
 * Customers can also return products.
 */
class Customer : public User {
protected:
    [[nodiscard]] UserType getUserType() const override;

public:
    explicit Customer(std::string name) : User(std::move(name)) {
        try {
            User::login();
            loggedInSuccessfully = true;
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to create a Customer: ", e.what());
        }
    };

    ~Customer() {
        if (loggedInSuccessfully) User::logout();
    };

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

    // Product related methods

    /**
     * Look in the `products` table.
     *
     * Products can be filtered by:
     *  - name
     *  - supplier_username
     *  - price (as a range; can omit the upper or lower bound to get all products with a price higher/lower than the given value)
     * Any combination of the above filters can be used, or none at all to get all products.
     *
     * Products can be sorted by:
     *  - name
     *  - supplier_username
     *  - price
     * Only one sorting method can be used at a time, or none at all to get the results in the order they were found.
     *
     * @param name The name of the product to filter for.
     * @param supplierUsername The username of the supplier to filter for.
     * @param priceLowerBound The lower bound of the price range to filter for.
     * @param priceUpperBound The upper bound of the price range to filter for.
     * @param orderBy The column to sort the results by. (Can be "name", "supplier_username" or "price")
     */
    void searchProduct(const std::optional<std::string> &name,
                       const std::optional<std::string> &supplierUsername,
                       const std::optional<uint32_t> &priceLowerBound,
                       const std::optional<uint32_t> &priceUpperBound,
                       const std::optional<std::string> &orderBy,
                       const std::optional<bool> &sortDescending) const;

    // Cart related methods

    /**
     * Add a product to the cart.
     * @param productId the id of the product to add
     * @param amount the amount of products to add. Defaults to 1.
     */
    void addProductToCart(const uint32_t &productId, const std::optional<uint32_t> &amount);

    /**
     * Remove a product from the cart.
     * @param productId the id of the product to remove.
     * @param amount the amount of products to remove. Defaults to the amount of the product in the cart.
     */
    void removeProductFromCart(const uint32_t &productId, const std::optional<uint32_t> &amount);

    /**
     * Get the contents of the cart.
     */
    void getCart() const;

    // Order related methods

    /**
     * Make an order from the products in the cart.
     * @param address the address to deliver the order to.
     */
    void makeOrder(const std::string &address);

    /**
     * Cancel an order.
     * @param orderId the id of the order to cancel.
     */
    void cancelOrder(const uint32_t &orderId);

    /**
     * Get the status of an order.
     * @param orderId the id of the order to get the status of.
     */
    void getOrderStatus(const uint32_t &orderId) const;

    /**
     * Get the history of orders.
     */
    void getOrdersHistory() const;

    /**
     * Make a return request for an order.
     * @param orderId the id of the order to return.
     * @param products a map of the products to return, with the key being the product id and the value being the quantity to return.
     */
    void returnProduct(const uint32_t &orderId, const std::map<uint32_t, uint32_t> &products) const;

    // TODO: A way to delete the Customer from the database, removing his account
};
