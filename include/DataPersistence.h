#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "exceptions.h"
#include "FootballLeague.h"
#include "StadiumShop.h"

// static utility for reading and writing the flat text files.
// doesn't own any objects — just reads from / writes to whatever you pass in.
class DataPersistence {
public:
    static void saveTeams(const FootballLeague& league, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for writing: " + filename);

        for (const auto* t : league.getTeams()) {
            file << t->getRegNumber() << "|"
                 << t->getName() << "|"
                 << t->getHomeCity() << "|"
                 << (t->isRegistered() ? "1" : "0") << "|"
                 << (t->isSuspended() ? "1" : "0") << "|"
                 << t->getPlayed() << "|"
                 << t->getWins() << "|"
                 << t->getDraws() << "|"
                 << t->getLosses() << "|"
                 << t->getGoalsFor() << "|"
                 << t->getGoalsAgainst() << "|"
                 << t->getPoints() << "\n";
        }
        file.close();
    }

    static void saveStadiums(const FootballLeague& league, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for writing: " + filename);

        for (const auto* s : league.getStadiums()) {
            file << s->getStadiumId() << "|"
                 << s->getName() << "|"
                 << s->getLocation() << "|"
                 << s->getSeatingCapacity() << "|"
                 << s->getFieldCondition() << "\n";
        }
        file.close();
    }

    static void saveShopInventory(const StadiumShop& shop, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for writing: " + filename);

        for (const auto& p : shop.getInventory()) {
            file << p->getCategoryType() << "|"
                 << p->getProductId() << "|"
                 << p->getName() << "|"
                 << p->getUnitPrice() << "|"
                 << p->getStockQuantity() << "\n";
        }
        file.close();
    }

    static void saveTransactions(const StadiumShop& shop, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for writing: " + filename);

        for (const auto& r : shop.getTransactionHistory()) {
            file << r.receiptId << "|"
                 << r.customerName << "|"
                 << r.subtotal << "|"
                 << r.gct << "|"
                 << r.totalAmount << "|"
                 << r.amountTendered << "|"
                 << r.change << "\n";
        }
        file.close();
    }

    static void loadTeams(FootballLeague& league, std::vector<std::unique_ptr<FootballTeam>>& teamStorage, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string regNo, name, city, regStr, suspStr;
            std::string pStr, wStr, dStr, lStr, gfStr, gaStr, ptsStr;

            std::getline(ss, regNo, '|');
            std::getline(ss, name, '|');
            std::getline(ss, city, '|');
            std::getline(ss, regStr, '|');
            std::getline(ss, suspStr, '|');

            auto team = std::make_unique<FootballTeam>(regNo, name, city);
            team->setRegistered(regStr == "1");
            team->setSuspended(suspStr == "1");

            league.addTeam(team.get());
            teamStorage.push_back(std::move(team));
        }
        file.close();
    }
};
