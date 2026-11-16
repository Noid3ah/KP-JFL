
#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>
using namespace std;

class Product {
private:
    string productId;
    string name;
    double price;
    int stockQuantity;

public:
    // Constructor
    Product(const string& productId,
            const string& name,
            double price,
            int stockQuantity);

    // Virtual destructor
    virtual ~Product();

    // Getters
    string getProductId() const;
    string getName() const;
    double getPrice() const;
    int getStockQuantity() const;

    // Setters
    void setName(const string& name);
    void setPrice(double price);
    void setStockQuantity(int quantity);

    // Abstract method
    virtual void displayDetails() const = 0;
};

#endif