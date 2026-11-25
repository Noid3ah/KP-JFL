#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include <map>
#include <algorithm>
#include "Product.h"
#include "Merchandise.h"
#include "FoodItem.h"
#include "exceptions.h"

using namespace std;

// one line item in a sale
struct SaleItem {
    string productId;
    string productName;
    int quantity;
    double unitPrice;
    double itemTotal;
};

// full receipt for a completed sale
struct Receipt {
    string receiptId;
    string customerName;
    vector<SaleItem> items;
    double subtotal;
    double gct;      ///< 15% Jamaican General Consumption Tax.
    double totalAmount;
    double amountTendered;
    double change;
};

// Manages the shop inventory and keeps a record of all completed sales.
// GCT is 15% — if payment falls short the stock gets put back and an exception is thrown.
class StadiumShop {
private:
    string shopId;
    string stadiumId;
    vector<unique_ptr<Product>> inventory;
    vector<Receipt> transactionHistory;
    int nextReceiptNo = 1;

    static constexpr double GCT_RATE = 0.15; // 15% GCT in Jamaica

public:
    StadiumShop(const string& shopId, const string& stadiumId)
        : shopId(shopId), stadiumId(stadiumId) {}

    const string& getShopId() const { return shopId; }
    const string& getStadiumId() const { return stadiumId; }
    const vector<Receipt>& getTransactionHistory() const { return transactionHistory; }

    void addProduct(unique_ptr<Product> product) {
        if (!product) throw invalid_argument("Product cannot be null");
        for (const auto& p : inventory) {
            if (p->getProductId() == product->getProductId()) {
                throw ShopFinanceException("Product ID " + product->getProductId() + " already exists in inventory");
            }
        }
        inventory.push_back(std::move(product));
    }

    Product* findProduct(const string& productId) {
        for (auto& p : inventory) {
            if (p->getProductId() == productId) return p.get();
        }
        return nullptr;
    }

    const vector<unique_ptr<Product>>& getInventory() const { return inventory; }

    // what a cart costs including GCT, worked out the same way as processSale
    double totalDue(const vector<pair<string, int>>& cart) {
        double subtotal = 0.0;
        for (const auto& item : cart) {
            const Product* p = findProduct(item.first);
            if (!p) throw ShopFinanceException("Product " + item.first + " not found in shop");
            const double itemTotal = p->getUnitPrice() * item.second;
            subtotal += itemTotal;
        }
        const double gct = subtotal * GCT_RATE;
        return subtotal + gct;
    }

    // handles a full sale — checks stock, deducts it, applies GCT, and rolls back if payment is short
    Receipt processSale(const string& customerName,
                        const vector<pair<string, int>>& cart,
                        double paymentTendered) {
        if (cart.empty()) {
            throw ShopFinanceException("Cart cannot be empty");
        }
        // '|' separates fields in the save file, so it can't appear inside one
        if ((customerName).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Customer name cannot contain '|' or line breaks.");
        }

        // check everything is in stock before touching any quantities
        // quantities are totalled per product, so listing one twice can't get past the check
        map<string, int> wanted;
        for (const auto& item : cart) {
            if (item.second <= 0) throw ShopFinanceException("Quantity for " + item.first + " must be positive");
            Product* p = findProduct(item.first);
            if (!p) {
                throw ShopFinanceException("Product " + item.first + " not found in shop");
            }
            // compared by subtraction so huge quantities cannot overflow
            if (item.second > p->getStockQuantity() - wanted[item.first]) {
                throw OutOfStockException("Insufficient stock for " + p->getName() + " (Available: "
                                          + to_string(p->getStockQuantity()) + ", requested: "
                                          + to_string(item.second) + ")");
            }
            wanted[item.first] += item.second;
        }

        // stock is confirmed — deduct it and tally up the subtotal
        double subtotal = 0.0;
        vector<SaleItem> soldItems;

        for (const auto& item : cart) {
            Product* p = findProduct(item.first);
            p->reduceStock(item.second);
            double itemTotal = p->getUnitPrice() * item.second;
            subtotal += itemTotal;
            soldItems.push_back({p->getProductId(), p->getName(), item.second, p->getUnitPrice(), itemTotal});
        }

        double gct = subtotal * GCT_RATE;
        double total = subtotal + gct;

        if (paymentTendered < total) {
            // put stock back since the sale didn't go through
            for (const auto& sold : soldItems) {
                Product* p = findProduct(sold.productId);
                if (p) p->addStock(sold.quantity);
            }
            throw InsufficientPaymentException(
                "Payment insufficient! Required: JMD $" + to_string(total)
                + " (Subtotal: $" + to_string(subtotal) + " + 15% GCT: $" + to_string(gct)
                + "), but customer tendered: JMD $" + to_string(paymentTendered));
        }

        double change = paymentTendered - total;

        Receipt receipt;
        receipt.receiptId = "RCP-" + to_string(nextReceiptNo++);
        receipt.customerName = customerName;
        receipt.items = soldItems;
        receipt.subtotal = subtotal;
        receipt.gct = gct;
        receipt.totalAmount = total;
        receipt.amountTendered = paymentTendered;
        receipt.change = change;

        transactionHistory.push_back(receipt);
        return receipt;
    }

    // puts back a saved receipt and continues numbering after it
    void restoreReceipt(const Receipt& r) {
        if (r.receiptId.size() < 5 || r.receiptId.compare(0, 4, "RCP-") != 0) {
            throw invalid_argument("bad receipt ID '" + r.receiptId + "'");
        }
        for (const auto& existing : transactionHistory) {
            if (existing.receiptId == r.receiptId) throw invalid_argument("duplicate receipt " + r.receiptId);
        }
        nextReceiptNo = max(nextReceiptNo, stoi(r.receiptId.substr(4)) + 1);
        transactionHistory.push_back(r);
    }

    void displayInventory() const {
        cout << "\n--- Stadium Shop Inventory (" << shopId << " @ " << stadiumId << ") ---\n";
        for (const auto& p : inventory) {
            p->displayProduct();
        }
        cout << "---------------------------------------------------------\n";
    }

    static void printReceipt(const Receipt& r) {
        cout << "\n========================================\n";
        cout << "             STADIUM SHOP RECEIPT        \n";
        cout << "Receipt ID : " << r.receiptId << "\n";
        cout << "Customer   : " << r.customerName << "\n";
        cout << "----------------------------------------\n";
        cout << left << setw(20) << "Item"
                  << right << setw(4) << "Qty"
                  << setw(8) << "Price"
                  << setw(8) << "Total" << "\n";

        for (const auto& item : r.items) {
            cout << left << setw(20) << item.productName
                      << right << setw(4) << item.quantity
                      << setw(8) << fixed << setprecision(2) << item.unitPrice
                      << setw(8) << item.itemTotal << "\n";
        }
        cout << "----------------------------------------\n";
        cout << left << setw(24) << "Subtotal:" << "JMD $" << setw(8) << r.subtotal << "\n";
        cout << left << setw(24) << "15% GCT (Govt Tax):" << "JMD $" << setw(8) << r.gct << "\n";
        cout << left << setw(24) << "TOTAL AMOUNT:" << "JMD $" << setw(8) << r.totalAmount << "\n";
        cout << left << setw(24) << "Cash Tendered:" << "JMD $" << setw(8) << r.amountTendered << "\n";
        cout << left << setw(24) << "Change Due:" << "JMD $" << setw(8) << r.change << "\n";
        cout << "========================================\n\n";
    }
};
