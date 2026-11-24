
#ifndef STADIUMSHOP_H
#define STADIUMSHOP_H

#include <string>
#include "Product.h"

using namespace std;

class StadiumShop {
private:
    static const int MAX_PRODUCTS = 100;

    Product* inventory[MAX_PRODUCTS];
    int productCount;

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

    // Customer transactions
    void processTransaction(string productIds[], int quantities[],
                            int numberOfItems, double payment);
    // File handling
    void saveInventory(string filename = "shop_inventory.csv");
    void loadInventory(string filename = "shop_inventory.csv");

    // Management reports
    void generateInventoryReport() const;
    void generateLowStockReport(int threshold) const;
    void generateInventoryValuationReport() const;

};



#endif
