
#include "FoodItem.h"
#include <iostream>
#include <stdexcept>

using namespace std;

// Constructor
FoodItem::FoodItem(string productId, string name,
                   double price, int stockQuantity,
                   string expiryDate, string foodCategory)
    : Product(productId, name, price, stockQuantity),
      expiryDate(expiryDate), foodCategory(foodCategory) {

    if (expiryDate.empty()) {
        throw invalid_argument("Expiry date cannot be empty.");
    }

    if (foodCategory.empty()) {
        throw invalid_argument("Food category cannot be empty.");
    }
}

// Getters
string FoodItem::getExpiryDate() const {
    return expiryDate;
}

string FoodItem::getFoodCategory() const {
    return foodCategory;
}

// Setters
void FoodItem::setExpiryDate(string expiryDate) {
    if (expiryDate.empty()) {
        throw invalid_argument("Expiry date cannot be empty.");
    }

    this->expiryDate = expiryDate;
}

void FoodItem::setFoodCategory(string foodCategory) {
    if (foodCategory.empty()) {
        throw invalid_argument("Food category cannot be empty.");
    }

    this->foodCategory = foodCategory;
}

// Display food item details
void FoodItem::displayDetails() const {
    cout << "\n--- Food Item Details ---" << endl;
    cout << "Product ID: " << getProductId() << endl;
    cout << "Name: " << getName() << endl;
    cout << "Food Category: " << foodCategory << endl;
    cout << "Expiry Date: " << expiryDate << endl;
    cout << "Price: $" << getPrice() << endl;
    cout << "Stock Quantity: " << getStockQuantity() << endl;
}
