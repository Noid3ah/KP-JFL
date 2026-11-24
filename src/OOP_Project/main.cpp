
#include <iostream>
#include <exception>

#include "Product.h"
#include "Merchandise.h"
#include "FoodItem.h"
#include "StadiumShop.h"

using namespace std;

int main() {

    StadiumShop shop;

    try {

        // Add merchandise
        shop.addProduct(new Merchandise(
            "MER001", "Jamaica Football Jersey",
            4500.00, 20, "Clothing", "Large"
        ));

        // Add food item
        shop.addProduct(new FoodItem(
            "FOOD001", "Beef Patty",
            250.00, 50, "2026-12-20", "Snack"
        ));

        // Display inventory
        shop.displayInventory();

        // Search for product
        cout << "\n--- Searching for Product ---" << endl;

        Product* found = shop.searchProduct("MER001");

        if (found != nullptr) {
            cout << "Found: " << found->getName() << endl;
        }

        // Update stock
        cout << "\n--- Updating Stock ---" << endl;

        shop.updateStock("MER001", 30);

        cout << "Updated stock: "
             << shop.searchProduct("MER001")->getStockQuantity()
             << endl;

        // Display inventory count
        cout << "\nProducts in shop: "
             << shop.getInventorySize() << endl;

        // Customer transaction
        cout << "\n--- Customer Transaction ---" << endl;

        string productIds[2] = {"MER001", "FOOD001"};
        int quantities[2] = {1, 2};

        shop.processTransaction(productIds, quantities, 2, 5500.00);

        // Display inventory after purchase
        cout << "\n--- Inventory After Purchase ---" << endl;

        shop.displayInventory();

        // Test insufficient payment
        cout << "\n--- Testing Insufficient Payment ---" << endl;

        try {
            shop.processTransaction(productIds, quantities, 2, 1000.00);
        }
        catch (const exception& e) {
            cout << "Transaction rejected: " << e.what() << endl;
        }

        // Confirm stock remains unchanged
        cout << "\n--- Checking Stock After Failed Transaction ---" << endl;

        shop.displayInventory();

        // Save current inventory to CSV
        cout << "\n--- Saving Inventory ---" << endl;

        shop.saveInventory();

        // Create another shop to test loading
        cout << "\n--- Testing Inventory Loading ---" << endl;

        StadiumShop loadedShop;

        loadedShop.loadInventory();

        // Display restored inventory
        cout << "\n--- Restored Inventory ---" << endl;

        loadedShop.displayInventory();

        cout << "\nOriginal inventory count: "
             << shop.getInventorySize() << endl;

        cout << "Loaded inventory count: "
             << loadedShop.getInventorySize() << endl;

        cout << "\n--- Testing Management Reports ---" << endl;

        shop.generateInventoryReport();
        shop.generateLowStockReport(30);
        shop.generateInventoryValuationReport();

        // Test the exception for an invalid threshold
        try {
            shop.generateLowStockReport(-5);
        }
        catch (const exception& e) {
            cout << "Report rejected: " << e.what() << endl;
        }

    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}
