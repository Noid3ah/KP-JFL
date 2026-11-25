#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include "Product.h"

/**
 * =========================================================================
 * CLASS: Merchandise (Person 4 - Subclass of Product)
 * =========================================================================
 * PURPOSE:
 *  Represents wearable or collectible football club gear (jerseys, caps, scarves).
 * =========================================================================
 */
class Merchandise : public Product {
private:
    std::string size;        ///< Clothing or item size, such as S, M, L, XL, or Standard.
    std::string teamRegNo;   ///< Team registration number, or "League" for general merchandise.
    std::string material;    ///< Material description such as Polyester or Cotton.

public:
    /// Creates club merchandise. Team and material values default to league-wide and Polyester.
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
