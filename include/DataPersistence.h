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
#include "SafeFile.h"
#include "FootballLeague.h"
#include "StadiumShop.h"

using namespace std;

// static utility for reading and writing the flat text files.
// doesn't own any objects — just reads from / writes to whatever you pass in.
class DataPersistence {
private:
    // keeps empty fields, including a trailing one
    static vector<string> split(const string& line, char sep) {
        vector<string> parts(1);
        for (char ch : line) {
            if (ch == sep) parts.emplace_back();
            else parts.back() += ch;
        }
        return parts;
    }

public:
    static void saveTeams(const FootballLeague& league, const string& filename) {
        stringstream file;

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
        writeFileSafely(file.str(), filename);
    }

    // format: teamId|playerId|name|age|gender|position|jersey|goals|status
    static void savePlayers(const FootballLeague& league, const string& filename) {
        stringstream out;
        for (const auto* t : league.getTeams()) {
            for (const auto& p : t->getSquad()) {
                out << t->getRegNumber() << "|" << p.getId() << "|" << p.getName() << "|"
                    << p.getAge() << "|" << p.getGender() << "|" << p.getPosition() << "|"
                    << p.getJerseyNumber() << "|" << p.getGoals() << "|" << p.getPlayingStatus() << "\n";
            }
        }

        // keep saved players whose team isn't loaded this session, so they aren't lost
        ifstream existing(filename);
        string line;
        while (getline(existing, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            string teamId = line.substr(0, line.find('|'));
            bool loaded = false;
            for (const auto* t : league.getTeams()) {
                if (t->getRegNumber() == teamId) loaded = true;
            }
            if (!loaded) out << line << "\n";
        }
        existing.close();

        writeFileSafely(out.str(), filename);
    }

    // replaces every team's squad with the saved one
    // the whole file is checked first, so a bad line changes nothing
    static void loadPlayers(FootballLeague& league, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        // scratch teams, so addPlayer/updatePlayer apply the usual squad rules
        map<string, FootballTeam> scratch;
        for (const auto* t : league.getTeams()) {
            scratch.emplace(t->getRegNumber(), FootballTeam(t->getRegNumber(), t->getName(), t->getHomeCity()));
        }

        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 9) throw invalid_argument("expected 9 fields");

                auto it = scratch.find(f[0]);
                if (it == scratch.end()) continue; // team not loaded; savePlayers keeps the line

                Player p(f[1], f[2], stoi(f[3]), f[4], f[5], stoi(f[6]));
                p.addGoals(stoi(f[7]));
                it->second.addPlayer(p);
                it->second.updatePlayer(p.getId(), p.getName(), p.getAge(), p.getPosition(), p.getJerseyNumber(), f[8]);
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }

        for (const auto& entry : scratch) {
            league.findTeam(entry.first)->getSquad() = entry.second.getSquad();
        }
    }

    // format: teamId|coachId|name|age|gender|qualification|role|yearsExperience
    static void saveCoaches(const FootballLeague& league, const string& filename) {
        stringstream out;
        for (const auto* t : league.getTeams()) {
            for (const auto& c : t->getCoachingStaff()) {
                out << t->getRegNumber() << "|" << c.getId() << "|" << c.getName() << "|" << c.getAge() << "|"
                    << c.getGender() << "|" << c.getQualification() << "|" << c.getCoachingRole() << "|"
                    << c.getYearsExperience() << "\n";
            }
        }
        writeFileSafely(out.str(), filename);
    }

    // replaces every team's coaching staff with the saved one
    static void loadCoaches(FootballLeague& league, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        for (auto* t : league.getTeams()) t->getCoachingStaff().clear();

        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 8) throw invalid_argument("expected 8 fields");
                FootballTeam* team = league.findTeam(f[0]);
                if (!team) throw invalid_argument("unknown team " + f[0]);
                team->addCoach(Coach(f[1], f[2], stoi(f[3]), f[4], f[5], f[6], stoi(f[7])));
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: id|name|age|gender|email|phone|preferredTeam
    // ticket counts aren't stored; loadTickets works them out from the sold tickets
    static void saveSpectators(const FootballLeague& league, const string& filename) {
        stringstream out;
        for (const auto* s : league.getSpectators()) {
            out << s->getId() << "|" << s->getName() << "|" << s->getAge() << "|" << s->getGender() << "|"
                << s->getEmail() << "|" << s->getPhone() << "|" << s->getPreferredTeam() << "\n";
        }
        writeFileSafely(out.str(), filename);
    }

    // spectators already in the league (the sample ones) are left alone; storage owns the added ones
    static void loadSpectators(FootballLeague& league, vector<unique_ptr<Spectator>>& storage, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        set<string> seeded;
        for (const auto* s : league.getSpectators()) seeded.insert(s->getId());

        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 7) throw invalid_argument("expected 7 fields");
                if (seeded.count(f[0])) continue;
                auto spectator = make_unique<Spectator>(f[0], f[1], stoi(f[2]), f[3], f[4], f[5], f[6]);
                league.addSpectator(spectator.get());
                storage.push_back(std::move(spectator));
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: fixtureId|ticketId|seat|category|price|spectatorId (blank if unsold)
    static void saveTickets(const FootballLeague& league, const string& filename) {
        stringstream out;
        out << fixed << setprecision(2);
        for (const auto& fx : league.getFixtures()) {
            for (const auto& t : fx->getTickets()) {
                out << fx->getFixtureId() << "|" << t.getTicketId() << "|" << t.getSeatNumber() << "|"
                    << ticketCategoryToString(t.getCategory()) << "|" << t.getPrice() << "|" << t.getSpectatorId() << "\n";
            }
        }
        writeFileSafely(out.str(), filename);
    }

    // call after fixtures and spectators are loaded; replaces every fixture's tickets
    static void loadTickets(FootballLeague& league, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        map<string, Fixture*> fixtures;
        map<string, Spectator*> spectators;
        for (const auto& fx : league.getFixtures()) fixtures[fx->getFixtureId()] = fx.get();
        for (auto* s : league.getSpectators()) {
            spectators[s->getId()] = s;
            s->resetTickets();
        }

        map<string, vector<Ticket>> loaded;
        set<string> seen;
        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 6) throw invalid_argument("expected 6 fields");
                const auto fx = fixtures.find(f[0]);
                if (fx == fixtures.end()) throw invalid_argument("unknown fixture " + f[0]);
                if (!seen.insert(f[0] + "|" + f[1]).second) throw invalid_argument("duplicate ticket " + f[1]);

                Ticket ticket(f[1], f[0], fx->second->getStadium().getStadiumId(), f[2],
                              ticketCategoryFromString(f[3]), stod(f[4]));
                if (!f[5].empty()) {
                    const auto buyer = spectators.find(f[5]);
                    if (buyer == spectators.end()) throw invalid_argument("unknown spectator " + f[5]);
                    ticket.sellTo(*buyer->second);
                }
                loaded[f[0]].push_back(ticket);
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }

        for (const auto& entry : fixtures) entry.second->restoreTickets(loaded[entry.first]);
    }

    static void saveStadiums(const FootballLeague& league, const string& filename) {
        stringstream file;

        for (const auto* s : league.getStadiums()) {
            file << s->getStadiumId() << "|"
                 << s->getName() << "|"
                 << s->getLocation() << "|"
                 << s->getSeatingCapacity() << "|"
                 << s->getFieldCondition() << "\n";
        }
        writeFileSafely(file.str(), filename);
    }

    // updates matching stadiums and adds the rest; storage owns the added ones
    static void loadStadiums(FootballLeague& league, vector<unique_ptr<Stadium>>& storage, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 5) throw invalid_argument("expected 5 fields");
                auto stadium = make_unique<Stadium>(f[0], f[1], f[2], stoi(f[3]), f[4]);
                if (Stadium* existing = league.findStadium(f[0])) {
                    existing->setLocation(stadium->getLocation());
                    existing->setSeatingCapacity(stadium->getSeatingCapacity());
                    existing->setFieldCondition(stadium->getFieldCondition());
                } else {
                    league.addStadium(stadium.get());
                    storage.push_back(std::move(stadium));
                }
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: id|date|time|homeId|awayId|stadiumId|officialId,officialId|status|homeScore|awayScore
    // the scores are blank until the match is completed
    static void saveFixtures(const FootballLeague& league, const string& filename) {
        stringstream out;
        for (const auto& fx : league.getFixtures()) {
            out << fx->getFixtureId() << "|" << fx->getDate() << "|" << fx->getTime() << "|"
                << fx->getHome().getRegNumber() << "|" << fx->getAway().getRegNumber() << "|"
                << fx->getStadium().getStadiumId() << "|";
            for (size_t i = 0; i < fx->getOfficials().size(); ++i) {
                out << (i ? "," : "") << fx->getOfficials()[i]->getId();
            }
            if (fx->getStatus() == FixtureStatus::COMPLETED) {
                out << "|COMPLETED|" << fx->getResult()->getHomeScore() << "|" << fx->getResult()->getAwayScore() << "\n";
            } else {
                out << "|SCHEDULED||\n";
            }
        }
        writeFileSafely(out.str(), filename);
    }

    // call after teams and stadiums are loaded, since fixtures refer to them
    // fixtures already in the league (the completed sample ones) are left alone
    static void loadFixtures(FootballLeague& league, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        set<string> seeded, seen;
        for (const auto& fx : league.getFixtures()) seeded.insert(fx->getFixtureId());

        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 10) throw invalid_argument("expected 10 fields");
                if (!seen.insert(f[0]).second) throw invalid_argument("duplicate fixture " + f[0]);
                if (seeded.count(f[0])) continue;

                FootballTeam* home = league.findTeam(f[3]);
                FootballTeam* away = league.findTeam(f[4]);
                Stadium* stadium = league.findStadium(f[5]);
                if (!home || !away || !stadium) throw invalid_argument("unknown team or stadium");
                if (f[7] != "SCHEDULED" && f[7] != "COMPLETED") throw invalid_argument("unknown status '" + f[7] + "'");

                unique_ptr<MatchResult> result;
                if (f[7] == "COMPLETED") result = make_unique<MatchResult>(stoi(f[8]), stoi(f[9]));
                vector<string> officialIds;
                if (!f[6].empty()) officialIds = split(f[6], ',');
                league.restoreFixture(f[0], f[1], f[2], home, away, stadium, officialIds, result.get());
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: category|id|name|price|stock, then size|team|material or beverage|perishable|expiry
    static void saveShopInventory(const StadiumShop& shop, const string& filename) {
        stringstream out;
        out << fixed << setprecision(2);
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
        writeFileSafely(out.str(), filename);
    }

    // sets the stock of products already in the shop and adds the rest
    // older files have only the first 5 fields, so new products from those get default details
    static void loadShopInventory(StadiumShop& shop, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        set<string> seen;
        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 5 && f.size() != 8) throw invalid_argument("expected 5 or 8 fields");
                if (!seen.insert(f[1]).second) throw invalid_argument("duplicate product " + f[1]);
                const bool full = f.size() == 8;
                const double price = stod(f[3]);
                const int stock = stoi(f[4]);

                unique_ptr<Product> product;
                if (f[0] == "Merchandise") {
                    product = full ? make_unique<Merchandise>(f[1], f[2], price, stock, f[5], f[6], f[7])
                                   : make_unique<Merchandise>(f[1], f[2], price, stock, "Standard");
                } else if (f[0] == "FoodItem") {
                    product = full ? make_unique<FoodItem>(f[1], f[2], price, stock, f[5] == "1", f[6] == "1", f[7])
                                   : make_unique<FoodItem>(f[1], f[2], price, stock, false);
                } else {
                    throw invalid_argument("unknown category '" + f[0] + "'");
                }

                if (Product* existing = shop.findProduct(f[1])) {
                    const int change = stock - existing->getStockQuantity();
                    if (change > 0) existing->addStock(change);
                    if (change < 0) existing->reduceStock(-change);
                } else {
                    shop.addProduct(std::move(product));
                }
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // format: id|customer|subtotal|gct|total|tendered|change, then productId|name|quantity|price per item
    static void saveTransactions(const StadiumShop& shop, const string& filename) {
        stringstream out;
        out << fixed << setprecision(2);
        for (const auto& r : shop.getTransactionHistory()) {
            out << r.receiptId << "|" << r.customerName << "|" << r.subtotal << "|" << r.gct << "|"
                << r.totalAmount << "|" << r.amountTendered << "|" << r.change;
            for (const auto& item : r.items) {
                out << "|" << item.productId << "|" << item.productName << "|" << item.quantity << "|" << item.unitPrice;
            }
            out << "\n";
        }
        writeFileSafely(out.str(), filename);
    }

    // older files have no items, so those receipts load with totals only
    static void loadTransactions(StadiumShop& shop, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() < 7 || (f.size() - 7) % 4 != 0) throw invalid_argument("wrong number of fields");
                Receipt r{f[0], f[1], {}, stod(f[2]), stod(f[3]), stod(f[4]), stod(f[5]), stod(f[6])};
                for (size_t i = 7; i < f.size(); i += 4) {
                    const int quantity = stoi(f[i + 2]);
                    const double price = stod(f[i + 3]);
                    r.items.push_back({f[i], f[i + 1], quantity, price, price * quantity});
                }
                shop.restoreReceipt(r);
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // updates matching teams and adds the rest; teamStorage owns the added ones
    // the whole file is checked first, so a bad line changes nothing
    static void loadTeams(FootballLeague& league, vector<unique_ptr<FootballTeam>>& teamStorage, const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) throw PersistenceException("Cannot open file for reading: " + filename);

        vector<unique_ptr<FootballTeam>> loaded;
        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            try {
                const vector<string> f = split(line, '|');
                if (f.size() != 12) throw invalid_argument("expected 12 fields");
                for (const auto& other : loaded) {
                    if (other->getRegNumber() == f[0]) throw invalid_argument("duplicate team " + f[0]);
                }

                auto team = make_unique<FootballTeam>(f[0], f[1], f[2]);
                team->setRegistered(f[3] == "1");
                team->setSuspended(f[4] == "1");
                team->setRecord(stoi(f[5]), stoi(f[6]), stoi(f[7]), stoi(f[8]),
                                stoi(f[9]), stoi(f[10]), stoi(f[11]));
                loaded.push_back(std::move(team));
            } catch (const exception& e) {
                throw PersistenceException(filename + " line " + to_string(lineNo) + ": " + e.what());
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
