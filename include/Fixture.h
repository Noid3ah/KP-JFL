#pragma once
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "exceptions.h"
#include "FootballTeam.h"
#include "MatchOfficial.h"
#include "MatchResult.h"
#include "Stadium.h"
#include "Ticket.h"

using namespace std;

// tracks whether a match is still upcoming or has been played
enum class FixtureStatus { SCHEDULED, COMPLETED };

// checks that text matches a pattern where 'd' stands for any digit
inline bool matchesDigits(const string& text, const string& pattern) {
    if (text.size() != pattern.size()) return false;
    for (size_t i = 0; i < text.size(); ++i) {
        const bool digit = text[i] >= '0' && text[i] <= '9';
        if (pattern[i] == 'd' ? !digit : text[i] != pattern[i]) return false;
    }
    return true;
}

// a real calendar date as YYYY-MM-DD
inline bool isValidDate(const string& date) {
    if (!matchesDigits(date, "dddd-dd-dd")) return false;
    const int year = stoi(date.substr(0, 4));
    const int month = stoi(date.substr(5, 2));
    const int day = stoi(date.substr(8, 2));
    if (year < 1 || month < 1 || month > 12) return false;
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int days[] = {31, leap ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return day >= 1 && day <= days[month - 1];
}

// a 24-hour time as HH:MM
inline bool isValidTime(const string& time) {
    return matchesDigits(time, "dd:dd") && stoi(time.substr(0, 2)) < 24 && stoi(time.substr(3, 2)) < 60;
}

// One scheduled match — holds pointers to the two teams and the venue.
// Owns the result and ticket objects for this game.
class Fixture {
private:
    string fixtureId;
    string date;
    string time;
    FootballTeam* home;
    FootballTeam* away;
    Stadium* stadium;
    vector<MatchOfficial*> officials;
    FixtureStatus status;
    unique_ptr<MatchResult> result;
    vector<Ticket> tickets;

public:
    Fixture(const string& id, const string& date, const string& time,
            FootballTeam* home, FootballTeam* away, Stadium* stadium)
        : fixtureId(id),
          date(date),
          time(time),
          home(home),
          away(away),
          stadium(stadium),
          status(FixtureStatus::SCHEDULED) {
        if (home == nullptr || away == nullptr || stadium == nullptr) {
            throw invalid_argument("Fixture needs two teams and a stadium");
        }
        if (home == away) {
            throw invalid_argument("A team cannot play itself");
        }
        if (date.empty() || time.empty()) {
            throw invalid_argument("Fixture needs a date and time");
        }
        // '|' separates fields in the save file, so it can't appear inside one
        if ((date + time).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Fixture details cannot contain '|' or line breaks.");
        }

        // let the stadium know these teams are playing there
        stadium->assignTeam(home->getRegNumber());
        stadium->assignTeam(away->getRegNumber());
    }

    const string& getFixtureId() const { return fixtureId; }
    const string& getDate() const { return date; }
    const string& getTime() const { return time; }
    FootballTeam& getHome() const { return *home; }
    FootballTeam& getAway() const { return *away; }
    Stadium& getStadium() const { return *stadium; }
    FixtureStatus getStatus() const { return status; }
    const vector<MatchOfficial*>& getOfficials() const { return officials; }
    const MatchResult* getResult() const { return result.get(); }
    const vector<Ticket>& getTickets() const { return tickets; }

    bool sameSlot(const Fixture& other) const {
        return date == other.date && time == other.time;
    }

    bool involvesTeam(const FootballTeam* team) const {
        return team == home || team == away;
    }

    bool clashesWith(const Fixture& other) const {
        const bool sameTeam = involvesTeam(other.home) || involvesTeam(other.away);
        return sameSlot(other) && sameTeam;
    }

    void addOfficial(MatchOfficial* official) {
        if (official == nullptr) {
            throw invalid_argument("Official cannot be null");
        }
        if (status == FixtureStatus::COMPLETED) {
            throw LeagueException("Cannot assign an official to completed fixture " + fixtureId);
        }

        for (MatchOfficial* assigned : officials) {
            if (assigned == official) {
                throw LeagueException("Official " + official->getName() + " is already assigned to fixture " + fixtureId);
            }
        }

        if (!official->isFree(date, time)) {
            throw OfficialConflictException(official->getName() + " is not free on " + date + " at " + time);
        }

        official->assignSlot(date, time);
        officials.push_back(official);
    }

    bool hasSoldTickets() const {
        for (const auto& t : tickets) if (t.getIsSold()) return true;
        return false;
    }

    int ticketsLeft(TicketCategory cat) const {
        int left = 0;
        for (const auto& t : tickets) if (!t.getIsSold() && t.getCategory() == cat) ++left;
        return left;
    }

    // generates tickets per seating category — total can't exceed stadium cap
    void generateTickets(int bleacherCount, int grandstandCount, int vipCount) {
        int total = bleacherCount + grandstandCount + vipCount;
        if (total > stadium->getSeatingCapacity()) {
            throw CapacityExceededException("Total tickets requested (" + to_string(total)
                                            + ") exceeds stadium capacity (" + to_string(stadium->getSeatingCapacity()) + ")");
        }

        // regenerating would erase tickets people have already bought
        if (hasSoldTickets()) {
            throw StadiumTicketingException("Cannot regenerate tickets for " + fixtureId + ": some are already sold.");
        }

        tickets.clear();
        int tIndex = 1;
        for (int i = 0; i < bleacherCount; ++i) {
            tickets.emplace_back("TK" + to_string(tIndex++), fixtureId, stadium->getStadiumId(),
                                 "BL-" + to_string(i + 1), TicketCategory::BLEACHERS);
        }
        for (int i = 0; i < grandstandCount; ++i) {
            tickets.emplace_back("TK" + to_string(tIndex++), fixtureId, stadium->getStadiumId(),
                                 "GS-" + to_string(i + 1), TicketCategory::GRANDSTAND);
        }
        for (int i = 0; i < vipCount; ++i) {
            tickets.emplace_back("TK" + to_string(tIndex++), fixtureId, stadium->getStadiumId(),
                                 "VIP-" + to_string(i + 1), TicketCategory::VIP);
        }
    }

    // puts back saved tickets — no capacity check, since capacity may have changed after they were issued
    void restoreTickets(const vector<Ticket>& saved) { tickets = saved; }

    Ticket* sellTicket(TicketCategory cat, Spectator& spectator) {
        if (status == FixtureStatus::COMPLETED) {
            throw StadiumTicketingException("Cannot sell tickets for completed fixture " + fixtureId + ".");
        }

        int soldCount = 0;
        for (const auto& t : tickets) {
            if (t.getIsSold()) ++soldCount;
        }

        if (soldCount >= stadium->getSeatingCapacity()) {
            throw CapacityExceededException("Cannot sell ticket: Stadium " + stadium->getName() + " is sold out!");
        }

        for (auto& t : tickets) {
            if (!t.getIsSold() && t.getCategory() == cat) {
                t.sellTo(spectator);
                return &t;
            }
        }
        throw StadiumTicketingException("No available tickets left in category " + ticketCategoryToString(cat));
    }

    void setResult(const MatchResult& matchResult) {
        if (status == FixtureStatus::COMPLETED) {
            throw LeagueException("Fixture " + fixtureId + " is already completed");
        }
        result = make_unique<MatchResult>(matchResult);
    }

    void complete() {
        if (result == nullptr) {
            throw ResultRequiredException("Fixture " + fixtureId + " cannot be completed without a result");
        }
        status = FixtureStatus::COMPLETED;
    }

    void display() const {
        cout << fixtureId << " | " << date << " " << time << " | "
                  << home->getName() << " vs " << away->getName() << " @ " << stadium->getName()
                  << " | Officials: " << officials.size() << " | ";

        if (result != nullptr) {
            cout << result->getHomeScore() << "-" << result->getAwayScore() << " ";
        }

        if (status == FixtureStatus::COMPLETED) {
            cout << "[Completed]";
        } else {
            cout << "[Scheduled]";
        }

        int soldCount = 0;
        for (const auto& t : tickets) if (t.getIsSold()) ++soldCount;
        cout << " | Tickets Sold: " << soldCount << "/" << tickets.size() << "\n";
    }
};
