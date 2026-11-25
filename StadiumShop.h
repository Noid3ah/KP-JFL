#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include "Product.h"
#include "Merchandise.h"
#include "FoodItem.h"
#include "exceptions.h"

/// Details for one product line in a completed transaction.
struct SaleItem {
    std::string productId;
    std::string productName;
    int quantity;
    double unitPrice;
    double itemTotal;
};

/// Final summary of a completed stadium-shop sale.
struct Receipt {
    std::string receiptId;
    std::string customerName;
    std::vector<SaleItem> items;
    double subtotal;
    double gct;      ///< 15% Jamaican General Consumption Tax.
    double totalAmount;
    double amountTendered;
    double change;
};

/**
 * Owns the shop's product inventory and completed transaction history.
 *
 * Products are stored as polymorphic unique_ptr objects so FoodItem and
 * Merchandise instances remain usable through the common Product interface.
 * Sales calculate Jamaican 15% GCT and roll stock back if payment is insufficient.
 */
class StadiumShop {
private:
    std::string shopId;
    std::string stadiumId;
    std::vector<std::unique_ptr<Product>> inventory;
    std::vector<Receipt> transactionHistory;
    int nextReceiptNo = 1;

    static constexpr double GCT_RATE = 0.15; // 15% GCT in Jamaica

public:
    StadiumShop(const std::string& shopId, const std::string& stadiumId)
        : shopId(shopId), stadiumId(stadiumId) {}

    const std::string& getShopId() const { return shopId; }
    const std::string& getStadiumId() const { return stadiumId; }
    const std::vector<Receipt>& getTransactionHistory() const { return transactionHistory; }

    void addProduct(std::unique_ptr<Product> product) {
        if (!product) throw std::invalid_argument("Product cannot be null");
        for (const auto& p : inventory) {
            if (p->getProductId() == product->getProductId()) {
                throw ShopFinanceException("Product ID " + product->getProductId() + " already exists in inventory");
            }
        }
        inventory.push_back(std::move(product));
    }

    Product* findProduct(const std::string& productId) {
        for (auto& p : inventory) {
            if (p->getProductId() == productId) return p.get();
        }
        return nullptr;
    }

    const std::vector<std::unique_ptr<Product>>& getInventory() const { return inventory; }

    // Process customer transactions:
    // Calculates subtotal, 15% GCT, total payment, change and prevents insufficient payment
    Receipt processSale(const std::string& customerName,
                        const std::vector<std::pair<std::string, int>>& cart,
                        double paymentTendered) {
        if (cart.empty()) {
            throw ShopFinanceException("Cart cannot be empty");
        }

        // Validate stock first
        for (const auto& item : cart) {
            Product* p = findProduct(item.first);
            if (!p) {
                throw ShopFinanceException("Product " + item.first + " not found in shop");
            }
            if (p->getStockQuantity() < item.second) {
                throw OutOfStockException("Insufficient stock for " + p->getName() + " (Available: "
                                          + std::to_string(p->getStockQuantity()) + ", requested: "
                                          + std::to_string(item.second) + ")");
            }
        }

        // Deduct stock and compute subtotal
        double subtotal = 0.0;
        std::vector<SaleItem> soldItems;

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
            // Rollback stock
            for (const auto& sold : soldItems) {
                Product* p = findProduct(sold.productId);
                if (p) p->addStock(sold.quantity);
            }
            throw InsufficientPaymentException(
                "Payment insufficient! Required: JMD $" + std::to_string(total)
                + " (Subtotal: $" + std::to_string(subtotal) + " + 15% GCT: $" + std::to_string(gct)
                + "), but customer tendered: JMD $" + std::to_string(paymentTendered));
        }

        double change = paymentTendered - total;

        Receipt receipt;
        receipt.receiptId = "RCP-" + std::to_string(nextReceiptNo++);
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

    void displayInventory() const {
        std::cout << "\n--- Stadium Shop Inventory (" << shopId << " @ " << stadiumId << ") ---\n";
        for (const auto& p : inventory) {
            p->displayProduct();
        }
        std::cout << "---------------------------------------------------------\n";
    }

    static void printReceipt(const Receipt& r) {
        std::cout << "\n========================================\n";
        std::cout << "             STADIUM SHOP RECEIPT        \n";
        std::cout << "Receipt ID : " << r.receiptId << "\n";
        std::cout << "Customer   : " << r.customerName << "\n";
        std::cout << "----------------------------------------\n";
        std::cout << std::left << std::setw(20) << "Item"
                  << std::right << std::setw(4) << "Qty"
                  << std::setw(8) << "Price"
                  << std::setw(8) << "Total" << "\n";

        for (const auto& item : r.items) {
            std::cout << std::left << std::setw(20) << item.productName
                      << std::right << std::setw(4) << item.quantity
                      << std::setw(8) << std::fixed << std::setprecision(2) << item.unitPrice
                      << std::setw(8) << item.itemTotal << "\n";
        }
        std::cout << "----------------------------------------\n";
        std::cout << std::left << std::setw(24) << "Subtotal:" << "JMD $" << std::setw(8) << r.subtotal << "\n";
        std::cout << std::left << std::setw(24) << "15% GCT (Govt Tax):" << "JMD $" << std::setw(8) << r.gct << "\n";
        std::cout << std::left << std::setw(24) << "TOTAL AMOUNT:" << "JMD $" << std::setw(8) << r.totalAmount << "\n";
        std::cout << std::left << std::setw(24) << "Cash Tendered:" << "JMD $" << std::setw(8) << r.amountTendered << "\n";
        std::cout << std::left << std::setw(24) << "Change Due:" << "JMD $" << std::setw(8) << r.change << "\n";
        std::cout << "========================================\n\n";
    }
};
