
#ifndef MERCHANDISE_H
#define MERCHANDISE_H

#include "Product.h"
#include <string>

using namespace std;

class Merchandise : public Product {
private:
    string category;
    string size;

public:
    // Constructor
    Merchandise(string productId, string name,
                double price, int stockQuantity,
                string category, string size);

    // Getter methods
    string getCategory() const;
    string getSize() const;

    // Setter methods
    void setCategory(string category);
    void setSize(string size);

    // Override abstract method
    void displayDetails() const override;
};

#endif
