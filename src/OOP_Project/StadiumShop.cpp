
#include "StadiumShop.h"
#include "FoodItem.h"
#include "Merchandise.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

using namespace std;

// Constructor
StadiumShop::StadiumShop() {

    productCount = 0;

    for (int i = 0; i < MAX_PRODUCTS; i++) {
        inventory[i] = nullptr;
    }
}

// Destructor
StadiumShop::~StadiumShop() {

    for (int i = 0; i < productCount; i++) {
        delete inventory[i];
        inventory[i] = nullptr;
    }
}

// Add a product to inventory
void StadiumShop::addProduct(Product* product) {

    if (product == nullptr) {
        throw invalid_argument("Cannot add an empty product.");
    }

    if (searchProduct(product->getProductId()) != nullptr) {
        throw invalid_argument("Product ID already exists.");
    }

    if (productCount >= MAX_PRODUCTS) {
        throw runtime_error("Shop inventory is full.");
    }

    inventory[productCount] = product;
    productCount++;

    cout << product->getName()
         << " added successfully." << endl;
}

// Display all products
void StadiumShop::displayInventory() const {

    if (productCount == 0) {
        cout << "\nThe shop inventory is empty." << endl;
        return;
    }

    cout << "\n========== SHOP INVENTORY ==========" << endl;

    for (int i = 0; i < productCount; i++) {
        inventory[i]->displayDetails();
    }

    cout << "\nTotal products: " << productCount << endl;
}

// Search for a product by ID
Product* StadiumShop::searchProduct(string productId) const {

    for (int i = 0; i < productCount; i++) {

        if (inventory[i]->getProductId() == productId) {
            return inventory[i];
        }
    }

    return nullptr;
}

// Remove a product from inventory
void StadiumShop::removeProduct(string productId) {

    for (int i = 0; i < productCount; i++) {

        if (inventory[i]->getProductId() == productId) {

            cout << inventory[i]->getName()
                 << " removed successfully." << endl;

            delete inventory[i];

            // Shift remaining products left
            for (int j = i; j < productCount - 1; j++) {
                inventory[j] = inventory[j + 1];
            }

            productCount--;
            inventory[productCount] = nullptr;

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
    return productCount;
}

// Process customer transaction
void StadiumShop::processTransaction(string productIds[], int quantities[],
                                      int numberOfItems, double payment) {

    if (numberOfItems <= 0) {
        throw invalid_argument("Transaction must contain products.");
    }

    if (productIds == nullptr || quantities == nullptr) {
        throw invalid_argument("Invalid purchase information.");
    }

    if (payment < 0) {
        throw invalid_argument("Payment cannot be negative.");
    }

    double subtotal = 0;

    // Validate all products before processing payment
    for (int i = 0; i < numberOfItems; i++) {

        if (quantities[i] <= 0) {
            throw invalid_argument("Quantity must be greater than zero.");
        }

        // Prevent duplicate products in one transaction
        for (int j = 0; j < i; j++) {

            if (productIds[i] == productIds[j]) {
                throw invalid_argument("Duplicate product in transaction.");
            }
        }

        Product* product = searchProduct(productIds[i]);

        if (product == nullptr) {
            throw runtime_error("Product not found.");
        }

        if (quantities[i] > product->getStockQuantity()) {
            throw runtime_error("Insufficient stock.");
        }

        FoodItem* food = dynamic_cast<FoodItem*>(product);

        if (food != nullptr && food->isExpired()) {
            throw runtime_error("Expired food cannot be sold.");
        }

        subtotal += product->getPrice() * quantities[i];
    }

    // Calculate GCT included in prices
    double gct = subtotal * 15.0 / 115.0;
    double total = subtotal;

    if (payment < total) {
        throw runtime_error("Insufficient payment. Transaction cancelled.");
    }

    double change = payment - total;

    // Deduct stock after successful payment
    for (int i = 0; i < numberOfItems; i++) {

        Product* product = searchProduct(productIds[i]);

        product->setStockQuantity(
            product->getStockQuantity() - quantities[i]
        );
    }

    // Display receipt
    cout << fixed << setprecision(2);

    cout << "\n========== CUSTOMER RECEIPT ==========" << endl;

    for (int i = 0; i < numberOfItems; i++) {

        Product* product = searchProduct(productIds[i]);

        cout << product->getName()
             << " x " << quantities[i]
             << " = $" << product->getPrice() * quantities[i]
             << endl;
    }

    cout << "--------------------------------------" << endl;
    cout << "Subtotal (GCT included): $" << subtotal << endl;
    cout << "GCT included: $" << gct << endl;
    cout << "Total: $" << total << endl;
    cout << "Payment: $" << payment << endl;
    cout << "Change: $" << change << endl;

    cout << "======================================" << endl;
    cout << "Transaction completed successfully!" << endl;
}


void StadiumShop::saveInventory(string filename) {

    ofstream file(filename);

    if (!file.is_open()) {
        throw runtime_error("Unable to open inventory file for saving.");
    }

    // Write CSV headings
    file << "ProductID,Type,Name,Price,Stock,Extra1,Extra2\n";

    for (int i = 0; i < productCount; i++) {

        Product* product = inventory[i];

        // Check whether the product is Merchandise
        Merchandise* merchandise = dynamic_cast<Merchandise*>(product);

        if (merchandise != nullptr) {

            file << merchandise->getProductId() << ","
                 << "Merchandise" << ","
                 << merchandise->getName() << ","
                 << merchandise->getPrice() << ","
                 << merchandise->getStockQuantity() << ","
                 << merchandise->getCategory() << ","
                 << merchandise->getSize() << "\n";
        }

        else {

            // Check whether the product is FoodItem
            FoodItem* food = dynamic_cast<FoodItem*>(product);

            if (food != nullptr) {

                file << food->getProductId() << ","
                     << "FoodItem" << ","
                     << food->getName() << ","
                     << food->getPrice() << ","
                     << food->getStockQuantity() << ","
                     << food->getExpiryDate() << ","
                     << food->getFoodCategory() << "\n";
            }
        }
    }

    file.close();

    cout << "Shop inventory saved successfully." << endl;
}


void StadiumShop::loadInventory(string filename) {

    ifstream file(filename);

    if (!file.is_open()) {
        throw runtime_error("Unable to open inventory file for loading.");
    }

    string line;

    // Read CSV headings
    if (!getline(file, line) ||
        line != "ProductID,Type,Name,Price,Stock,Extra1,Extra2") {

        file.close();
        throw runtime_error("Invalid inventory file format.");
    }

    int loadedCount = 0;
    int skippedCount = 0;

    while (getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        stringstream ss(line);

        string productId;
        string type;
        string name;
        string priceText;
        string stockText;
        string extra1;
        string extra2;
        string extra;

        // Separate CSV columns
        getline(ss, productId, ',');
        getline(ss, type, ',');
        getline(ss, name, ',');
        getline(ss, priceText, ',');
        getline(ss, stockText, ',');
        getline(ss, extra1, ',');
        getline(ss, extra);

        // Check that all seven fields exist
        if (productId.empty() || type.empty() || name.empty() ||
            priceText.empty() || stockText.empty() ||
            extra1.empty() || extra2.empty()) {

            // extra2 is checked below using the actual parsed value
        }

        if (productId.empty() || type.empty() || name.empty() ||
            priceText.empty() || stockText.empty() ||
            extra1.empty() || extra.empty()) {

            cout << "Invalid record skipped." << endl;
            skippedCount++;
            continue;
        }

        try {

            double price = stod(priceText);
            int stock = stoi(stockText);

            // Prevent duplicate product IDs
            if (searchProduct(productId) != nullptr) {

                cout << "Duplicate product skipped: "
                     << productId << endl;

                skippedCount++;
                continue;
            }

            // Check inventory capacity
            if (productCount >= MAX_PRODUCTS) {

                cout << "Inventory full. Remaining records skipped."
                     << endl;

                skippedCount++;
                continue;
            }

            Product* product = nullptr;

            if (type == "Merchandise") {

                product = new Merchandise(
                    productId,
                    name,
                    price,
                    stock,
                    extra1,
                    extra
                );
            }

            else if (type == "FoodItem") {

                product = new FoodItem(
                    productId,
                    name,
                    price,
                    stock,
                    extra1,
                    extra
                );
            }

            else {

                cout << "Unknown product type skipped: "
                     << type << endl;

                skippedCount++;
                continue;
            }

            // Add reconstructed product to inventory
            addProduct(product);

            loadedCount++;
        }

        catch (const exception& e) {

            cout << "Invalid record skipped: "
                 << e.what() << endl;

            skippedCount++;
        }
    }

    file.close();

    cout << "\n========== INVENTORY LOADING ==========" << endl;
    cout << "Products loaded: " << loadedCount << endl;
    cout << "Records skipped: " << skippedCount << endl;
    cout << "=======================================" << endl;
}

// ---------- MANAGEMENT REPORTS ----------

void StadiumShop::generateInventoryReport() const {

    cout << "\n================ INVENTORY REPORT ================" << endl;

    if (productCount == 0) {
        cout << "The shop inventory is empty." << endl;
        cout << "==================================================" << endl;
        return;
    }

    cout << left << setw(10) << "ID"
         << setw(28) << "Name"
         << setw(14) << "Type"
         << right << setw(12) << "Price (J$)"
         << setw(8) << "Stock" << endl;
    cout << "--------------------------------------------------------------------" << endl;

    for (int i = 0; i < productCount; i++) {

        string type = "Other";

        if (dynamic_cast<Merchandise*>(inventory[i]) != nullptr) {
            type = "Merchandise";
        }
        else if (dynamic_cast<FoodItem*>(inventory[i]) != nullptr) {
            type = "Food Item";
        }

        cout << left << setw(10) << inventory[i]->getProductId()
             << setw(28) << inventory[i]->getName()
             << setw(14) << type
             << right << setw(12) << fixed << setprecision(2)
             << inventory[i]->getPrice()
             << setw(8) << inventory[i]->getStockQuantity() << endl;
    }

    cout << "--------------------------------------------------------------------" << endl;
    cout << "Total products: " << productCount << endl;
    cout << "==================================================" << endl;
}


void StadiumShop::generateLowStockReport(int threshold) const {

    if (threshold < 0) {
        throw invalid_argument("Low stock threshold cannot be negative.");
    }

    cout << "\n=============== LOW STOCK REPORT ===============" << endl;
    cout << "Products with stock below " << threshold << ":\n" << endl;

    int lowStockCount = 0;

    for (int i = 0; i < productCount; i++) {

        if (inventory[i]->getStockQuantity() < threshold) {

            cout << inventory[i]->getProductId() << " - "
                 << inventory[i]->getName()
                 << " (Stock: " << inventory[i]->getStockQuantity() << ")"
                 << endl;

            lowStockCount++;
        }
    }

    if (lowStockCount == 0) {
        cout << "No products are below the threshold." << endl;
    }
    else {
        cout << "\nProducts needing restock: " << lowStockCount << endl;
    }

    cout << "================================================" << endl;
}


void StadiumShop::generateInventoryValuationReport() const {

    cout << "\n========== INVENTORY VALUATION REPORT ==========" << endl;

    if (productCount == 0) {
        cout << "The shop inventory is empty. Total value: J$0.00" << endl;
        cout << "================================================" << endl;
        return;
    }

    double totalValue = 0.0;

    cout << left << setw(10) << "ID"
         << setw(28) << "Name"
         << right << setw(8) << "Stock"
         << setw(12) << "Price"
         << setw(14) << "Value" << endl;
    cout << "--------------------------------------------------------------------" << endl;

    for (int i = 0; i < productCount; i++) {

        double value = inventory[i]->getPrice() * inventory[i]->getStockQuantity();
        totalValue += value;

        cout << left << setw(10) << inventory[i]->getProductId()
             << setw(28) << inventory[i]->getName()
             << right << setw(8) << inventory[i]->getStockQuantity()
             << setw(12) << fixed << setprecision(2) << inventory[i]->getPrice()
             << setw(14) << value << endl;
    }

    cout << "--------------------------------------------------------------------" << endl;
    cout << "Total inventory value: J$" << fixed << setprecision(2)
         << totalValue << endl;
    cout << "(Prices include 15% GCT, so this is value at tax-inclusive retail price.)"
         << endl;
    cout << "================================================" << endl;
}