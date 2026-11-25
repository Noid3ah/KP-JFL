#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include "Product.h"

using namespace std;

// Match-day food and drinks sold at the shop — patties, Ting, jerk chicken, etc.
class FoodItem : public Product {
private:
    bool isBeverage;       ///< true for drinks; false for snacks or meals.
    bool isPerishable;     ///< Indicates whether the item may require expiry tracking.
    string expiryDate; ///< Display label for the item's expiry date.

public:
    // expiry defaults to "Today" if nothing is passed in
    FoodItem(const string& id, const string& name, double price, int stock,
             bool isBeverage, bool isPerishable = true, const string& expiry = "Today")
        : Product(id, name, price, stock, "FoodItem"),
          isBeverage(isBeverage), isPerishable(isPerishable), expiryDate(expiry) {
        // '|' separates fields in the save file, so it can't appear inside one
        if ((expiry).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Product details cannot contain '|' or line breaks.");
        }
    }

    bool getIsBeverage() const { return isBeverage; }
    bool getIsPerishable() const { return isPerishable; }
    const string& getExpiryDate() const { return expiryDate; }

    void displayProduct() const override {
        cout << "  [Food]  #" << productId << " | " << left << setw(24) << name
                  << " | Type: " << setw(8) << (isBeverage ? "Drink" : "Snack")
                  << " | Expiry: " << setw(6) << expiryDate
                  << " | Stock: " << right << setw(4) << stockQuantity
                  << " | Price: JMD $" << fixed << setprecision(2) << unitPrice << "\n";
    }
};
