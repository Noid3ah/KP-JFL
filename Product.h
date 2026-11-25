#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "exceptions.h"

// Base class for anything sold in the shop.
// Can't be used directly — subclasses (Merchandise, FoodItem) implement displayProduct().
class Product {
protected:
    std::string productId;  ///< Unique inventory identifier.
    std::string name;       ///< Customer-facing product name.
    double unitPrice;       ///< Price per unit in Jamaican dollars.
    int stockQuantity;      ///< Current number of units available.
    std::string categoryType; ///< "Merchandise" or "FoodItem".

public:
    Product(const std::string& id, const std::string& name, double price, int stock, const std::string& catType)
        : productId(id), name(name), unitPrice(price), stockQuantity(stock), categoryType(catType) {
        if (id.empty() || name.empty()) throw std::invalid_argument("Product ID and Name cannot be empty.");
        if (price < 0.0) throw std::invalid_argument("Product price cannot be negative.");
        if (stock < 0) throw std::invalid_argument("Stock quantity cannot be negative.");
    }

    virtual ~Product() = default;

    const std::string& getProductId() const { return productId; }
    const std::string& getName() const { return name; }
    double getUnitPrice() const { return unitPrice; }
    int getStockQuantity() const { return stockQuantity; }
    const std::string& getCategoryType() const { return categoryType; }

    void addStock(int qty) {
        if (qty <= 0) throw std::invalid_argument("Quantity to add must be positive.");
        stockQuantity += qty;
    }

    void reduceStock(int qty) {
        if (qty <= 0) throw std::invalid_argument("Quantity must be positive.");
        if (qty > stockQuantity) {
            throw OutOfStockException("Insufficient stock for " + name + " (Requested: "
                                      + std::to_string(qty) + ", Available: " + std::to_string(stockQuantity) + ").");
        }
        stockQuantity -= qty;
    }

    // each subclass formats its own display line
    virtual void displayProduct() const = 0;
};
