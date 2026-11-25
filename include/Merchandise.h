#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include "Product.h"

using namespace std;

// Club gear sold in the shop — jerseys, caps, scarves, etc.
// Linked to a specific team or to the league in general.
class Merchandise : public Product {
private:
    string size;        ///< Clothing or item size, such as S, M, L, XL, or Standard.
    string teamRegNo;   ///< Team registration number, or "League" for general merchandise.
    string material;    ///< Material description such as Polyester or Cotton.

public:
    // team defaults to "League" and material to Polyester if not specified
    Merchandise(const string& id, const string& name, double price, int stock,
                const string& size, const string& teamRegNo = "League",
                const string& material = "Polyester")
        : Product(id, name, price, stock, "Merchandise"),
          size(size), teamRegNo(teamRegNo), material(material) {
        // '|' separates fields in the save file, so it can't appear inside one
        if ((size + teamRegNo + material).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Product details cannot contain '|' or line breaks.");
        }
    }

    const string& getSize() const { return size; }
    const string& getTeamRegNo() const { return teamRegNo; }
    const string& getMaterial() const { return material; }

    void displayProduct() const override {
        cout << "  [Merch] #" << productId << " | " << left << setw(24) << name
                  << " | Size: " << setw(8) << size
                  << " | Team: " << setw(8) << teamRegNo
                  << " | Stock: " << right << setw(4) << stockQuantity
                  << " | Price: JMD $" << fixed << setprecision(2) << unitPrice << "\n";
    }
};
