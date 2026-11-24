#pragma once
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "exceptions.h"
#include "Fixture.h"
#include "LeagueCommissioner.h"

class FootballLeague {
private:
    std::string leagueName;
    LeagueCommissioner* commissioner = nullptr;
    std::vector<FootballTeam*> teams;
    std::vector<Stadium*> stadiums;
    std::vector<MatchOfficial*> officials;
    std::vector<std::unique_ptr<Fixture>> fixtures;
    int nextFixtureNo = 1;

    Fixture& findFixture(const std::string& id) {
        for (auto& fixture : fixtures) {
            if (fixture->getFixtureId() == id) {
                return *fixture;
            }
        }

        throw LeagueException("Fixture not found: " + id);
    }

    MatchOfficial& findOfficial(const std::string& id) {
        for (MatchOfficial* official : officials) {
            if (official->getId() == id) {
                return *official;
            }
        }

        throw LeagueException("Official not found: " + id);
    }

    static void checkEligible(const FootballTeam* t) {
        if (!t->isRegistered()) {
            throw TeamNotEligibleException(t->getName() + " is not registered");
        }

        if (t->isSuspended()) {
            throw TeamNotEligibleException(t->getName() + " is suspended");
        }
    }

public:
    explicit FootballLeague(const std::string& name)
        : leagueName(name) {
        if (name.empty()) {
            throw std::invalid_argument("League name cannot be empty");
        }
    }

    void setCommissioner(LeagueCommissioner* leagueCommissioner) {
        if (leagueCommissioner == nullptr) {
            throw std::invalid_argument("League commissioner cannot be null");
        }
        commissioner = leagueCommissioner;
    }

    void addTeam(FootballTeam* team) {
        if (team == nullptr) {
            throw std::invalid_argument("Team cannot be null");
        }
        for (FootballTeam* existing : teams) {
            if (existing == team
                || existing->getRegNumber() == team->getRegNumber()) {
                throw LeagueException("Team is already registered in this league");
            }
        }
        teams.push_back(team);
    }

    void addStadium(Stadium* stadium) {
        if (stadium == nullptr) {
            throw std::invalid_argument("Stadium cannot be null");
        }
        for (Stadium* existing : stadiums) {
            if (existing == stadium
                || existing->getStadiumId() == stadium->getStadiumId()) {
                throw LeagueException("Stadium is already registered in this league");
            }
        }
        stadiums.push_back(stadium);
    }

    void addOfficial(MatchOfficial* official) {
        if (official == nullptr) {
            throw std::invalid_argument("Official cannot be null");
        }
        for (MatchOfficial* existing : officials) {
            if (existing == official || existing->getId() == official->getId()) {
                throw LeagueException("Official is already registered in this league");
            }
        }
        officials.push_back(official);
    }

    Fixture& scheduleFixture(FootballTeam* home, FootballTeam* away, Stadium* stadium,
                             const std::string& date, const std::string& time) {
        if (home == nullptr || away == nullptr) {
            throw std::invalid_argument("Fixture needs two teams");
        }

        checkEligible(home);
        checkEligible(away);

        std::ostringstream fixtureId;
        fixtureId << "FX" << std::setw(3) << std::setfill('0') << nextFixtureNo;
        auto newFixture = std::make_unique<Fixture>(
            fixtureId.str(), date, time, home, away, stadium);

        for (const auto& existingFixture : fixtures) {
            if (newFixture->clashesWith(*existingFixture)) {
                throw SchedulingConflictException(
                    "A team is already booked on " + date + " at " + time
                    + " (" + existingFixture->getFixtureId() + ")");
            }

            const bool sameStadiumAndTime =
                newFixture->sameSlot(*existingFixture)
                && &existingFixture->getStadium() == stadium;
            if (sameStadiumAndTime) {
                throw SchedulingConflictException(
                    stadium->getName() + " is already booked on " + date + " at " + time
                    + " (" + existingFixture->getFixtureId() + ")");
            }
        }

        ++nextFixtureNo;
        fixtures.push_back(std::move(newFixture));
        return *fixtures.back();
    }

    void assignOfficial(const std::string& fixtureId, const std::string& officialId) {
        Fixture& fixture = findFixture(fixtureId);
        MatchOfficial& official = findOfficial(officialId);
        fixture.addOfficial(&official);
    }

    void recordResult(const std::string& fixtureId, int homeScore, int awayScore) {
        Fixture& fixture = findFixture(fixtureId);
        if (fixture.getStatus() == FixtureStatus::COMPLETED) {
            throw LeagueException("Fixture " + fixtureId + " already has a result");
        }

        MatchResult result(homeScore, awayScore);
        fixture.setResult(result);
        fixture.complete();

        fixture.getHome().recordMatch(homeScore, awayScore, result.getHomePoints());
        fixture.getAway().recordMatch(awayScore, homeScore, result.getAwayPoints());
    }

    std::vector<FootballTeam*> getStandings() const {
        std::vector<FootballTeam*> table = teams;
        std::sort(table.begin(), table.end(),
                  [](const FootballTeam* firstTeam, const FootballTeam* secondTeam) {
                      if (firstTeam->getPoints() != secondTeam->getPoints()) {
                          return firstTeam->getPoints() > secondTeam->getPoints();
                      }
                      if (firstTeam->getGoalDifference() != secondTeam->getGoalDifference()) {
                          return firstTeam->getGoalDifference() > secondTeam->getGoalDifference();
                      }
                      if (firstTeam->getGoalsFor() != secondTeam->getGoalsFor()) {
                          return firstTeam->getGoalsFor() > secondTeam->getGoalsFor();
                      }
                      return firstTeam->getName() < secondTeam->getName();
                  });

        return table;
    }

    void displayStandings() const {
        std::cout << "\n" << leagueName << " standings\n";
        std::cout << std::left << std::setw(4) << "Pos" << std::setw(18) << "Team"
                  << std::right << std::setw(4) << "P" << std::setw(4) << "W" << std::setw(4) << "D"
                  << std::setw(4) << "L" << std::setw(5) << "GD" << std::setw(5) << "Pts" << "\n";

        int pos = 1;
        for (const FootballTeam* team : getStandings()) {
            std::cout << std::left << std::setw(4) << pos++
                      << std::setw(18) << team->getName()
                      << std::right << std::setw(4) << team->getPlayed()
                      << std::setw(4) << team->getWins()
                      << std::setw(4) << team->getDraws()
                      << std::setw(4) << team->getLosses()
                      << std::setw(5) << team->getGoalDifference()
                      << std::setw(5) << team->getPoints() << "\n";
        }
    }

    void displayFixtures() const {
        std::cout << "\nFixtures\n";
        for (const auto& fixture : fixtures) {
            fixture->display();
        }
    }
};
