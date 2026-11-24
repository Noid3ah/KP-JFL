
#ifndef FOODITEM_H
#define FOODITEM_H

#include "Product.h"
#include <string>

using namespace std;

class FoodItem : public Product {
private:
    string expiryDate;
    string foodCategory;

public:
    // Constructor
    FoodItem(string productId, string name,
             double price, int stockQuantity,
             string expiryDate, string foodCategory);

    // Getters
    string getExpiryDate() const;
    string getFoodCategory() const;

    // Setters
    void setExpiryDate(string expiryDate);
    void setFoodCategory(string foodCategory);

    // Override abstract method
    void displayDetails() const override;
};

#endif
