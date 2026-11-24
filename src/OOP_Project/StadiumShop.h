
#ifndef STADIUMSHOP_H
#define STADIUMSHOP_H

#include <vector>
#include <string>
#include "Product.h"

using namespace std;

class StadiumShop {
private:
    vector<Product*> inventory;

public:
    // Constructor and destructor
    StadiumShop();
    ~StadiumShop();

    // Inventory management
    void addProduct(Product* product);
    void displayInventory() const;
    Product* searchProduct(string productId) const;
    void removeProduct(string productId);
    void updateStock(string productId, int quantity);

    // Inventory information
    int getInventorySize() const;

    
};

#endif
