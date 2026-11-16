
#include "StadiumShop.h"
#include <iostream>
#include <stdexcept>

using namespace std;

// Constructor
StadiumShop::StadiumShop() {
}

// Destructor
StadiumShop::~StadiumShop() {
    for (Product* product : inventory) {
        delete product;
    }

    inventory.clear();
}

// Add a product to inventory
void StadiumShop::addProduct(Product* product) {

    if (product == nullptr) {
        throw invalid_argument("Cannot add an empty product.");
    }

    // Check for duplicate product IDs
    if (searchProduct(product->getProductId()) != nullptr) {
        throw invalid_argument("Product ID already exists.");
    }

    inventory.push_back(product);

    cout << product->getName()
         << " added successfully." << endl;
}

// Display all products
void StadiumShop::displayInventory() const {

    if (inventory.empty()) {
        cout << "\nThe shop inventory is empty." << endl;
        return;
    }

    cout << "\n========== SHOP INVENTORY ==========" << endl;

    for (Product* product : inventory) {
        product->displayDetails();
    }

    cout << "\nTotal products: " << inventory.size() << endl;
}

// Search for a product by ID
Product* StadiumShop::searchProduct(string productId) const {

    for (Product* product : inventory) {

        if (product->getProductId() == productId) {
            return product;
        }
    }

    return nullptr;
}

// Remove a product from inventory
void StadiumShop::removeProduct(string productId) {

    for (auto it = inventory.begin(); it != inventory.end(); ++it) {

        if ((*it)->getProductId() == productId) {

            cout << (*it)->getName()
                 << " removed successfully." << endl;

            delete *it;
            inventory.erase(it);

            return;
        }
    }

    cout << "Product not found." << endl;
}

// Update product stock
void StadiumShop::updateStock(string productId, int quantity) {

    if (quantity < 0) {
        throw invalid_argument("Stock quantity cannot be negative.");
    }

    Product* product = searchProduct(productId);

    if (product == nullptr) {
        cout << "Product not found." << endl;
        return;
    }

    product->setStockQuantity(quantity);

    cout << "Stock updated successfully." << endl;
}

// Get the number of products in inventory
int StadiumShop::getInventorySize() const {
    return static_cast<int>(inventory.size());
}
