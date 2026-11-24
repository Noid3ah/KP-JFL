
#include <iostream>
#include "Product.h"
#include "Merchandise.h"
#include "FoodItem.h"
#include "StadiumShop.h"

using namespace std;

int main() {

    // Create shop
    StadiumShop shop;

    // Add merchandise
    shop.addProduct(new Merchandise(
        "MER001", "Jamaica Football Jersey",
        4500.00, 20, "Clothing", "Large"
    ));

    // Add food item
    shop.addProduct(new FoodItem(
        "FOOD001", "Beef Patty",
        250.00, 50, "2026-11-20", "Snack"
    ));

    // Display inventory
    shop.displayInventory();

    // Search for a product
    cout << "\n--- Searching for Product ---" << endl;

    Product* found = shop.searchProduct("MER001");

    if (found != nullptr) {
        cout << "Found: " << found->getName() << endl;
    } else {
        cout << "Product not found." << endl;
    }

    // Update stock
    cout << "\n--- Updating Stock ---" << endl;
    shop.updateStock("MER001", 30);

    // Display updated stock
    cout << "Updated stock: "
         << shop.searchProduct("MER001")->getStockQuantity()
         << endl;

    // Display inventory count
    cout << "\nProducts in shop: "
         << shop.getInventorySize() << endl;

    // Remove product
    cout << "\n--- Removing Product ---" << endl;
    shop.removeProduct("FOOD001");

    // Display final inventory
    shop.displayInventory();

    return 0;
}

