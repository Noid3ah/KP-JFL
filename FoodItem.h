#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include "Product.h"

/**
 * =========================================================================
 * CLASS: FoodItem (Person 4 - Subclass of Product)
 * =========================================================================
 * PURPOSE:
 *  Represents match-day concessions (Tastee patties, Ting soda, Jerk chicken).
 * =========================================================================
 */
class FoodItem : public Product {
private:
    bool isBeverage;       ///< true for drinks; false for snacks or meals.
    bool isPerishable;     ///< Indicates whether the item may require expiry tracking.
    std::string expiryDate; ///< Display label for the item's expiry date.

public:
    /// Creates a concession item, using a perishable "Today" default if no expiry is supplied.
    FoodItem(const std::string& id, const std::string& name, double price, int stock,
             bool isBeverage, bool isPerishable = true, const std::string& expiry = "Today")
        : Product(id, name, price, stock, "FoodItem"),
          isBeverage(isBeverage), isPerishable(isPerishable), expiryDate(expiry) {}

    /// Returns whether this item is a beverage rather than a snack or meal.
    bool getIsBeverage() const { return isBeverage; }

    /// Returns whether the item is marked as perishable.
    bool getIsPerishable() const { return isPerishable; }

    /// Returns the stored expiry-date label.
    const std::string& getExpiryDate() const { return expiryDate; }

    void displayProduct() const override {
        std::cout << "  [Food]  #" << productId << " | " << std::left << std::setw(24) << name
                  << " | Type: " << std::setw(8) << (isBeverage ? "Drink" : "Snack")
                  << " | Expiry: " << std::setw(6) << expiryDate
                  << " | Stock: " << std::right << std::setw(4) << stockQuantity
                  << " | Price: JMD $" << std::fixed << std::setprecision(2) << unitPrice << "\n";
    }
};
