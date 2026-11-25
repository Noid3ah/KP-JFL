#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include "Product.h"

// Club gear sold in the shop — jerseys, caps, scarves, etc.
// Linked to a specific team or to the league in general.
class Merchandise : public Product {
private:
    std::string size;        ///< Clothing or item size, such as S, M, L, XL, or Standard.
    std::string teamRegNo;   ///< Team registration number, or "League" for general merchandise.
    std::string material;    ///< Material description such as Polyester or Cotton.

public:
    // team defaults to "League" and material to Polyester if not specified
    Merchandise(const std::string& id, const std::string& name, double price, int stock,
                const std::string& size, const std::string& teamRegNo = "League",
                const std::string& material = "Polyester")
        : Product(id, name, price, stock, "Merchandise"),
          size(size), teamRegNo(teamRegNo), material(material) {}

    const std::string& getSize() const { return size; }
    const std::string& getTeamRegNo() const { return teamRegNo; }
    const std::string& getMaterial() const { return material; }

    void displayProduct() const override {
        std::cout << "  [Merch] #" << productId << " | " << std::left << std::setw(24) << name
                  << " | Size: " << std::setw(8) << size
                  << " | Team: " << std::setw(8) << teamRegNo
                  << " | Stock: " << std::right << std::setw(4) << stockQuantity
                  << " | Price: JMD $" << std::fixed << std::setprecision(2) << unitPrice << "\n";
    }
};
