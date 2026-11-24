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

enum class FixtureStatus { SCHEDULED, COMPLETED };

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
    }

    const std::string& getFixtureId() const { return fixtureId; }
    const std::string& getDate() const { return date; }
    const std::string& getTime() const { return time; }
    FootballTeam& getHome() const { return *home; }
    FootballTeam& getAway() const { return *away; }
    Stadium& getStadium() const { return *stadium; }
    FixtureStatus getStatus() const { return status; }
    const std::vector<MatchOfficial*>& getOfficials() const {
        return officials;
    }

    const MatchResult* getResult() const {
        return result.get();
    }

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
                throw LeagueException("Official " + official->getName()
                                      + " is already assigned to fixture " + fixtureId);
            }
        }

        if (!official->isFree(date, time)) {
            throw OfficialConflictException(official->getName() + " is not free on "
                                            + date + " at " + time);
        }

        official->assignSlot(date, time);
        officials.push_back(official);
    }

    void setResult(const MatchResult& matchResult) {
        if (status == FixtureStatus::COMPLETED) {
            throw LeagueException("Fixture " + fixtureId + " is already completed");
        }

        result = std::make_unique<MatchResult>(matchResult);
    }

    void complete() {
        if (result == nullptr) {
            throw ResultRequiredException("Fixture " + fixtureId
                                          + " cannot be completed without a result");
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

        std::cout << "\n";
    }
};
