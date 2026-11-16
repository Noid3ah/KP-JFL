
#include "Product.h"
#include <iostream>
#include <stdexcept>
using namespace std;

// Constructor
Product::Product(const string& productId,
                 const string& name,
                 double price,
                 int stockQuantity)
    : productId(productId), name(name),
      price(price), stockQuantity(stockQuantity) {

    if (productId.empty()) {
        throw invalid_argument("Product ID cannot be empty.");
    }

    if (name.empty()) {
        throw invalid_argument("Product name cannot be empty.");
    }

    if (price < 0) {
        throw invalid_argument("Price cannot be negative.");
    }

    if (stockQuantity < 0) {
        throw invalid_argument("Stock quantity cannot be negative.");
    }
}

// Destructor
Product::~Product() {}

// Getters
string Product::getProductId() const {
    return productId;
}

string Product::getName() const {
    return name;
}

double Product::getPrice() const {
    return price;
}

int Product::getStockQuantity() const {
    return stockQuantity;
}

// Setters
void Product::setName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Product name cannot be empty.");
    }

    this->name = name;
}

void Product::setPrice(double price) {
    if (price < 0) {
        throw std::invalid_argument("Price cannot be negative.");
    }

    this->price = price;
}

void Product::setStockQuantity(int quantity) {
    if (quantity < 0) {
        throw std::invalid_argument("Stock quantity cannot be negative.");
    }

    stockQuantity = quantity;
}