#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "exceptions.h"

using namespace std;

// Base class for anything sold in the shop.
// Can't be used directly — subclasses (Merchandise, FoodItem) implement displayProduct().
class Product {
protected:
    string productId;  ///< Unique inventory identifier.
    string name;       ///< Customer-facing product name.
    double unitPrice;       ///< Price per unit in Jamaican dollars.
    int stockQuantity;      ///< Current number of units available.
    string categoryType; ///< "Merchandise" or "FoodItem".

public:
    Product(const string& id, const string& name, double price, int stock, const string& catType)
        : productId(id), name(name), unitPrice(price), stockQuantity(stock), categoryType(catType) {
        if (id.empty() || name.empty()) throw invalid_argument("Product ID and Name cannot be empty.");
        if (price < 0.0) throw invalid_argument("Product price cannot be negative.");
        if (stock < 0) throw invalid_argument("Stock quantity cannot be negative.");
        // '|' separates fields in the save file, so it can't appear inside one
        if ((id + name).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Product details cannot contain '|' or line breaks.");
        }
    }

    virtual ~Product() = default;

    const string& getProductId() const { return productId; }
    const string& getName() const { return name; }
    double getUnitPrice() const { return unitPrice; }
    int getStockQuantity() const { return stockQuantity; }
    const string& getCategoryType() const { return categoryType; }

    void addStock(int qty) {
        if (qty <= 0) throw invalid_argument("Quantity to add must be positive.");
        stockQuantity += qty;
    }

    void reduceStock(int qty) {
        if (qty <= 0) throw invalid_argument("Quantity must be positive.");
        if (qty > stockQuantity) {
            throw OutOfStockException("Insufficient stock for " + name + " (Requested: "
                                      + to_string(qty) + ", Available: " + to_string(stockQuantity) + ").");
        }
        stockQuantity -= qty;
    }

    // each subclass formats its own display line
    virtual void displayProduct() const = 0;
};
