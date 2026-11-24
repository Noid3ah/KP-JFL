
#include "Merchandise.h"
#include <iostream>
#include <stdexcept>

using namespace std;

// Constructor
Merchandise::Merchandise(string productId, string name,
                         double price, int stockQuantity,
                         string category, string size)
    : Product(productId, name, price, stockQuantity),
      category(category), size(size) {

    if (category.empty()) {
        throw invalid_argument("Category cannot be empty.");
    }

    if (size.empty()) {
        throw invalid_argument("Size cannot be empty.");
    }
}

// Getters
string Merchandise::getCategory() const {
    return category;
}

string Merchandise::getSize() const {
    return size;
}

// Setters
void Merchandise::setCategory(string category) {
    if (category.empty()) {
        throw invalid_argument("Category cannot be empty.");
    }

    this->category = category;
}

void Merchandise::setSize(string size) {
    if (size.empty()) {
        throw invalid_argument("Size cannot be empty.");
    }

    this->size = size;
}

// Display product details
void Merchandise::displayDetails() const {
    cout << "\n--- Merchandise Details ---" << endl;
    cout << "Product ID: " << getProductId() << endl;
    cout << "Name: " << getName() << endl;
    cout << "Category: " << category << endl;
    cout << "Size: " << size << endl;
    cout << "Price: $" << getPrice() << endl;
    cout << "Stock Quantity: " << getStockQuantity() << endl;
}
