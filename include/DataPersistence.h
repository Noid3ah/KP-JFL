#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>
#include <vector>
#include <string>
#include "exceptions.h"
#include "FootballLeague.h"
#include "StadiumShop.h"

// static utility for reading and writing the flat text files.
// doesn't own any objects — just reads from / writes to whatever you pass in.
class DataPersistence {
private:
    // keeps empty fields, including a trailing one
    static std::vector<std::string> split(const std::string& line, char sep) {
        std::vector<std::string> parts(1);
        for (char ch : line) {
            if (ch == sep) parts.emplace_back();
            else parts.back() += ch;
        }
        return parts;
    }

    static void writeFile(const std::string& content, const std::string& filename) {
        // write to a temp file first so a failed write can't damage the saved data
        const std::string temp = filename + ".tmp";
        std::ofstream file(temp);
        if (!file.is_open()) throw PersistenceException("Cannot open file for writing: " + temp);
        file << content;
        file.close();
        if (!file) throw PersistenceException("Failed writing: " + temp);
        std::filesystem::rename(temp, filename);
    }

public:
    static void saveTeams(const FootballLeague& league, const std::string& filename) {
        std::stringstream file;

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
        writeFile(file.str(), filename);
    }

    // format: teamId|playerId|name|age|gender|position|jersey|goals|status
    static void savePlayers(const FootballLeague& league, const std::string& filename) {
        std::stringstream out;
        for (const auto* t : league.getTeams()) {
            for (const auto& p : t->getSquad()) {
                out << t->getRegNumber() << "|" << p.getId() << "|" << p.getName() << "|"
                    << p.getAge() << "|" << p.getGender() << "|" << p.getPosition() << "|"
                    << p.getJerseyNumber() << "|" << p.getGoals() << "|" << p.getPlayingStatus() << "\n";
            }
        }

        // keep saved players whose team isn't loaded this session, so they aren't lost
        std::ifstream existing(filename);
        std::string line;
        while (std::getline(existing, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            std::string teamId = line.substr(0, line.find('|'));
            bool loaded = false;
            for (const auto* t : league.getTeams()) {
                if (t->getRegNumber() == teamId) loaded = true;
            }
            if (!loaded) out << line << "\n";
        }
        existing.close();

        writeFile(out.str(), filename);
    }

    // replaces every team's squad with the saved one
    // the whole file is checked first, so a bad line changes nothing
    static void loadPlayers(FootballLeague& league, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        // scratch teams, so addPlayer/updatePlayer apply the usual squad rules
        std::map<std::string, FootballTeam> scratch;
        for (const auto* t : league.getTeams()) {
            scratch.emplace(t->getRegNumber(), FootballTeam(t->getRegNumber(), t->getName(), t->getHomeCity()));
        }

        std::string line;
        for (int lineNo = 1; std::getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const std::vector<std::string> f = split(line, '|');
                if (f.size() != 9) throw std::invalid_argument("expected 9 fields");

                auto it = scratch.find(f[0]);
                if (it == scratch.end()) continue; // team not loaded; savePlayers keeps the line

                Player p(f[1], f[2], std::stoi(f[3]), f[4], f[5], std::stoi(f[6]));
                p.addGoals(std::stoi(f[7]));
                it->second.addPlayer(p);
                it->second.updatePlayer(p.getId(), p.getName(), p.getAge(), p.getPosition(), p.getJerseyNumber(), f[8]);
            } catch (const std::exception& e) {
                throw PersistenceException(filename + " line " + std::to_string(lineNo) + ": " + e.what());
            }
        }

        for (const auto& entry : scratch) {
            league.findTeam(entry.first)->getSquad() = entry.second.getSquad();
        }
    }

    static void saveStadiums(const FootballLeague& league, const std::string& filename) {
        std::stringstream file;

        for (const auto* s : league.getStadiums()) {
            file << s->getStadiumId() << "|"
                 << s->getName() << "|"
                 << s->getLocation() << "|"
                 << s->getSeatingCapacity() << "|"
                 << s->getFieldCondition() << "\n";
        }
        writeFile(file.str(), filename);
    }

    // updates matching stadiums and adds the rest; storage owns the added ones
    static void loadStadiums(FootballLeague& league, std::vector<std::unique_ptr<Stadium>>& storage, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        std::string line;
        for (int lineNo = 1; std::getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const std::vector<std::string> f = split(line, '|');
                if (f.size() != 5) throw std::invalid_argument("expected 5 fields");
                auto stadium = std::make_unique<Stadium>(f[0], f[1], f[2], std::stoi(f[3]), f[4]);
                if (Stadium* existing = league.findStadium(f[0])) {
                    existing->setLocation(stadium->getLocation());
                    existing->setSeatingCapacity(stadium->getSeatingCapacity());
                    existing->setFieldCondition(stadium->getFieldCondition());
                } else {
                    league.addStadium(stadium.get());
                    storage.push_back(std::move(stadium));
                }
            } catch (const std::exception& e) {
                throw PersistenceException(filename + " line " + std::to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: id|date|time|homeId|awayId|stadiumId|officialId,officialId|status|homeScore|awayScore
    // the scores are blank until the match is completed
    static void saveFixtures(const FootballLeague& league, const std::string& filename) {
        std::stringstream out;
        for (const auto& fx : league.getFixtures()) {
            out << fx->getFixtureId() << "|" << fx->getDate() << "|" << fx->getTime() << "|"
                << fx->getHome().getRegNumber() << "|" << fx->getAway().getRegNumber() << "|"
                << fx->getStadium().getStadiumId() << "|";
            for (std::size_t i = 0; i < fx->getOfficials().size(); ++i) {
                out << (i ? "," : "") << fx->getOfficials()[i]->getId();
            }
            if (fx->getStatus() == FixtureStatus::COMPLETED) {
                out << "|COMPLETED|" << fx->getResult()->getHomeScore() << "|" << fx->getResult()->getAwayScore() << "\n";
            } else {
                out << "|SCHEDULED||\n";
            }
        }
        writeFile(out.str(), filename);
    }

    // call after teams and stadiums are loaded, since fixtures refer to them
    // fixtures already in the league (the completed sample ones) are left alone
    static void loadFixtures(FootballLeague& league, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        std::set<std::string> seeded, seen;
        for (const auto& fx : league.getFixtures()) seeded.insert(fx->getFixtureId());

        std::string line;
        for (int lineNo = 1; std::getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const std::vector<std::string> f = split(line, '|');
                if (f.size() != 10) throw std::invalid_argument("expected 10 fields");
                if (!seen.insert(f[0]).second) throw std::invalid_argument("duplicate fixture " + f[0]);
                if (seeded.count(f[0])) continue;

                FootballTeam* home = league.findTeam(f[3]);
                FootballTeam* away = league.findTeam(f[4]);
                Stadium* stadium = league.findStadium(f[5]);
                if (!home || !away || !stadium) throw std::invalid_argument("unknown team or stadium");
                if (f[7] != "SCHEDULED" && f[7] != "COMPLETED") throw std::invalid_argument("unknown status '" + f[7] + "'");

                std::unique_ptr<MatchResult> result;
                if (f[7] == "COMPLETED") result = std::make_unique<MatchResult>(std::stoi(f[8]), std::stoi(f[9]));
                std::vector<std::string> officialIds;
                if (!f[6].empty()) officialIds = split(f[6], ',');
                league.restoreFixture(f[0], f[1], f[2], home, away, stadium, officialIds, result.get());
            } catch (const std::exception& e) {
                throw PersistenceException(filename + " line " + std::to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: category|id|name|price|stock, then size|team|material or beverage|perishable|expiry
    static void saveShopInventory(const StadiumShop& shop, const std::string& filename) {
        std::stringstream out;
        out << std::fixed << std::setprecision(2);
        for (const auto& p : shop.getInventory()) {
            out << p->getCategoryType() << "|" << p->getProductId() << "|" << p->getName() << "|"
                << p->getUnitPrice() << "|" << p->getStockQuantity();
            if (const auto* m = dynamic_cast<const Merchandise*>(p.get())) {
                out << "|" << m->getSize() << "|" << m->getTeamRegNo() << "|" << m->getMaterial();
            } else if (const auto* d = dynamic_cast<const FoodItem*>(p.get())) {
                out << "|" << d->getIsBeverage() << "|" << d->getIsPerishable() << "|" << d->getExpiryDate();
            }
            out << "\n";
        }
        writeFile(out.str(), filename);
    }

    // sets the stock of products already in the shop and adds the rest
    // older files have only the first 5 fields, so new products from those get default details
    static void loadShopInventory(StadiumShop& shop, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        std::set<std::string> seen;
        std::string line;
        for (int lineNo = 1; std::getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const std::vector<std::string> f = split(line, '|');
                if (f.size() != 5 && f.size() != 8) throw std::invalid_argument("expected 5 or 8 fields");
                if (!seen.insert(f[1]).second) throw std::invalid_argument("duplicate product " + f[1]);
                const bool full = f.size() == 8;
                const double price = std::stod(f[3]);
                const int stock = std::stoi(f[4]);

                std::unique_ptr<Product> product;
                if (f[0] == "Merchandise") {
                    product = full ? std::make_unique<Merchandise>(f[1], f[2], price, stock, f[5], f[6], f[7])
                                   : std::make_unique<Merchandise>(f[1], f[2], price, stock, "Standard");
                } else if (f[0] == "FoodItem") {
                    product = full ? std::make_unique<FoodItem>(f[1], f[2], price, stock, f[5] == "1", f[6] == "1", f[7])
                                   : std::make_unique<FoodItem>(f[1], f[2], price, stock, false);
                } else {
                    throw std::invalid_argument("unknown category '" + f[0] + "'");
                }

                if (Product* existing = shop.findProduct(f[1])) {
                    const int change = stock - existing->getStockQuantity();
                    if (change > 0) existing->addStock(change);
                    if (change < 0) existing->reduceStock(-change);
                } else {
                    shop.addProduct(std::move(product));
                }
            } catch (const std::exception& e) {
                throw PersistenceException(filename + " line " + std::to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: id|customer|subtotal|gct|total|tendered|change, then productId|name|quantity|price per item
    static void saveTransactions(const StadiumShop& shop, const std::string& filename) {
        std::stringstream out;
        out << std::fixed << std::setprecision(2);
        for (const auto& r : shop.getTransactionHistory()) {
            out << r.receiptId << "|" << r.customerName << "|" << r.subtotal << "|" << r.gct << "|"
                << r.totalAmount << "|" << r.amountTendered << "|" << r.change;
            for (const auto& item : r.items) {
                out << "|" << item.productId << "|" << item.productName << "|" << item.quantity << "|" << item.unitPrice;
            }
            out << "\n";
        }
        writeFile(out.str(), filename);
    }

    // older files have no items, so those receipts load with totals only
    static void loadTransactions(StadiumShop& shop, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        std::string line;
        for (int lineNo = 1; std::getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const std::vector<std::string> f = split(line, '|');
                if (f.size() < 7 || (f.size() - 7) % 4 != 0) throw std::invalid_argument("wrong number of fields");
                Receipt r{f[0], f[1], {}, std::stod(f[2]), std::stod(f[3]), std::stod(f[4]), std::stod(f[5]), std::stod(f[6])};
                for (std::size_t i = 7; i < f.size(); i += 4) {
                    const int quantity = std::stoi(f[i + 2]);
                    const double price = std::stod(f[i + 3]);
                    r.items.push_back({f[i], f[i + 1], quantity, price, price * quantity});
                }
                shop.restoreReceipt(r);
            } catch (const std::exception& e) {
                throw PersistenceException(filename + " line " + std::to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // updates matching teams and adds the rest; teamStorage owns the added ones
    // the whole file is checked first, so a bad line changes nothing
    static void loadTeams(FootballLeague& league, std::vector<std::unique_ptr<FootballTeam>>& teamStorage, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        std::vector<std::unique_ptr<FootballTeam>> loaded;
        std::string line;
        for (int lineNo = 1; std::getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const std::vector<std::string> f = split(line, '|');
                if (f.size() != 12) throw std::invalid_argument("expected 12 fields");
                for (const auto& other : loaded) {
                    if (other->getRegNumber() == f[0]) throw std::invalid_argument("duplicate team " + f[0]);
                }

                auto team = std::make_unique<FootballTeam>(f[0], f[1], f[2]);
                team->setRegistered(f[3] == "1");
                team->setSuspended(f[4] == "1");
                team->setRecord(std::stoi(f[5]), std::stoi(f[6]), std::stoi(f[7]), std::stoi(f[8]),
                                std::stoi(f[9]), std::stoi(f[10]), std::stoi(f[11]));
                loaded.push_back(std::move(team));
            } catch (const std::exception& e) {
                throw PersistenceException(filename + " line " + std::to_string(lineNo) + ": " + e.what());
            }
        }

        for (auto& team : loaded) {
            if (FootballTeam* existing = league.findTeam(team->getRegNumber())) {
                // personnel aren't in this file, so carry them over
                team->getSquad() = existing->getSquad();
                team->getCoachingStaff() = existing->getCoachingStaff();
                *existing = *team;
            } else {
                league.addTeam(team.get());
                teamStorage.push_back(std::move(team));
            }
        }
    }
};
