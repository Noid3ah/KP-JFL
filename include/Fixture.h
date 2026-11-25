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

// tracks whether a match is still upcoming or has been played
enum class FixtureStatus { SCHEDULED, COMPLETED };

// One scheduled match — holds pointers to the two teams and the venue.
// Owns the result and ticket objects for this game.
class Fixture {
private:
    std::string fixtureId;
    std::string date;
    std::string time;
    FootballTeam* home;
    FootballTeam* away;
    Stadium* stadium;
    std::vector<MatchOfficial*> officials;
    FixtureStatus status;
    std::unique_ptr<MatchResult> result;
    std::vector<Ticket> tickets;

public:
    Fixture(const std::string& id, const std::string& date, const std::string& time,
            FootballTeam* home, FootballTeam* away, Stadium* stadium)
        : fixtureId(id),
          date(date),
          time(time),
          home(home),
          away(away),
          stadium(stadium),
          status(FixtureStatus::SCHEDULED) {
        if (home == nullptr || away == nullptr || stadium == nullptr) {
            throw std::invalid_argument("Fixture needs two teams and a stadium");
        }
        if (home == away) {
            throw std::invalid_argument("A team cannot play itself");
        }
        if (date.empty() || time.empty()) {
            throw std::invalid_argument("Fixture needs a date and time");
        }
        // '|' separates fields in the save file, so it can't appear inside one
        if ((date + time).find_first_of("|\r\n") != std::string::npos) {
            throw std::invalid_argument("Fixture details cannot contain '|' or line breaks.");
        }

        // let the stadium know these teams are playing there
        stadium->assignTeam(home->getRegNumber());
        stadium->assignTeam(away->getRegNumber());
    }

    const std::string& getFixtureId() const { return fixtureId; }
    const std::string& getDate() const { return date; }
    const std::string& getTime() const { return time; }
    FootballTeam& getHome() const { return *home; }
    FootballTeam& getAway() const { return *away; }
    Stadium& getStadium() const { return *stadium; }
    FixtureStatus getStatus() const { return status; }
    const std::vector<MatchOfficial*>& getOfficials() const { return officials; }
    const MatchResult* getResult() const { return result.get(); }
    const std::vector<Ticket>& getTickets() const { return tickets; }

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
            throw std::invalid_argument("Official cannot be null");
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

    // generates tickets per seating category — total can't exceed stadium cap
    void generateTickets(int bleacherCount, int grandstandCount, int vipCount) {
        int total = bleacherCount + grandstandCount + vipCount;
        if (total > stadium->getSeatingCapacity()) {
            throw CapacityExceededException("Total tickets requested (" + std::to_string(total)
                                            + ") exceeds stadium capacity (" + std::to_string(stadium->getSeatingCapacity()) + ")");
        }

        // regenerating would erase tickets people have already bought
        for (const auto& t : tickets) {
            if (t.getIsSold()) {
                throw StadiumTicketingException("Cannot regenerate tickets for " + fixtureId + ": some are already sold.");
            }
        }

        tickets.clear();
        int tIndex = 1;
        for (int i = 0; i < bleacherCount; ++i) {
            tickets.emplace_back("TK" + std::to_string(tIndex++), fixtureId, stadium->getStadiumId(),
                                 "BL-" + std::to_string(i + 1), TicketCategory::BLEACHERS);
        }
        for (int i = 0; i < grandstandCount; ++i) {
            tickets.emplace_back("TK" + std::to_string(tIndex++), fixtureId, stadium->getStadiumId(),
                                 "GS-" + std::to_string(i + 1), TicketCategory::GRANDSTAND);
        }
        for (int i = 0; i < vipCount; ++i) {
            tickets.emplace_back("TK" + std::to_string(tIndex++), fixtureId, stadium->getStadiumId(),
                                 "VIP-" + std::to_string(i + 1), TicketCategory::VIP);
        }
    }

    // puts back saved tickets — no capacity check, since capacity may have changed after they were issued
    void restoreTickets(const std::vector<Ticket>& saved) { tickets = saved; }

    Ticket* sellTicket(TicketCategory cat, Spectator& spectator) {
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
        result = std::make_unique<MatchResult>(matchResult);
    }

    void complete() {
        if (result == nullptr) {
            throw ResultRequiredException("Fixture " + fixtureId + " cannot be completed without a result");
        }
        status = FixtureStatus::COMPLETED;
    }

    void display() const {
        std::cout << fixtureId << " | " << date << " " << time << " | "
                  << home->getName() << " vs " << away->getName() << " @ " << stadium->getName()
                  << " | Officials: " << officials.size() << " | ";

        if (result != nullptr) {
            std::cout << result->getHomeScore() << "-" << result->getAwayScore() << " ";
        }

        if (status == FixtureStatus::COMPLETED) {
            std::cout << "[Completed]";
        } else {
            std::cout << "[Scheduled]";
        }

        int soldCount = 0;
        for (const auto& t : tickets) if (t.getIsSold()) ++soldCount;
        std::cout << " | Tickets Sold: " << soldCount << "/" << tickets.size() << "\n";
    }
};
