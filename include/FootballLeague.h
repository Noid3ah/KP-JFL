#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include "exceptions.h"
#include "Fixture.h"
#include "LeagueCommissioner.h"
#include "FootballTeam.h"
#include "Stadium.h"
#include "MatchOfficial.h"
#include "Spectator.h"

// The main league class — ties together teams, stadiums, officials, fixtures, and standings.
// Owns fixture objects but only holds references to everything else.
class FootballLeague {
private:
    std::string leagueName;
    LeagueCommissioner* commissioner = nullptr;
    std::vector<FootballTeam*> teams;
    std::vector<Stadium*> stadiums;
    std::vector<MatchOfficial*> officials;
    std::vector<Spectator*> spectators;
    std::vector<std::unique_ptr<Fixture>> fixtures;
    int nextFixtureNo = 1;

    // looks up a fixture — throws if the ID doesn't match anything
    Fixture& findFixture(const std::string& id) {
        for (auto& f : fixtures) {
            if (f->getFixtureId() == id) return *f;
        }
        throw LeagueException("Fixture not found with ID: " + id);
    }

    // looks up an official — throws if not found
    MatchOfficial& findOfficial(const std::string& id) {
        for (auto* o : officials) {
            if (o->getId() == id) return *o;
        }
        throw LeagueException("Match official not found with ID: " + id);
    }

    // blocks unregistered or suspended teams from being scheduled
    static void checkEligibility(const FootballTeam* team) {
        if (!team->isRegistered()) {
            throw TeamNotEligibleException("Team '" + team->getName() + "' is not registered with the league.");
        }
        if (team->isSuspended()) {
            throw TeamNotEligibleException("Team '" + team->getName() + "' is currently suspended.");
        }
    }

public:
    explicit FootballLeague(const std::string& name) : leagueName(name) {
        if (name.empty()) throw std::invalid_argument("League name cannot be empty.");
    }

    // getters
    const std::string& getLeagueName() const { return leagueName; }
    LeagueCommissioner* getCommissioner() const { return commissioner; }
    const std::vector<FootballTeam*>& getTeams() const { return teams; }
    const std::vector<Stadium*>& getStadiums() const { return stadiums; }
    const std::vector<MatchOfficial*>& getOfficials() const { return officials; }
    const std::vector<Spectator*>& getSpectators() const { return spectators; }
    const std::vector<std::unique_ptr<Fixture>>& getFixtures() const { return fixtures; }

    void setCommissioner(LeagueCommissioner* comm) {
        if (!comm) throw std::invalid_argument("Commissioner pointer cannot be null.");
        commissioner = comm;
    }

    void addTeam(FootballTeam* team) {
        if (!team) throw std::invalid_argument("Team pointer cannot be null.");
        for (const auto* existing : teams) {
            if (existing->getRegNumber() == team->getRegNumber()) {
                throw LeagueException("Team " + team->getRegNumber() + " is already in the league.");
            }
        }
        teams.push_back(team);
    }

    void addStadium(Stadium* stadium) {
        if (!stadium) throw std::invalid_argument("Stadium pointer cannot be null.");
        for (const auto* s : stadiums) {
            if (s->getStadiumId() == stadium->getStadiumId()) {
                throw LeagueException("Stadium " + stadium->getStadiumId() + " already exists.");
            }
        }
        stadiums.push_back(stadium);
    }

    void addOfficial(MatchOfficial* official) {
        if (!official) throw std::invalid_argument("Official pointer cannot be null.");
        for (const auto* o : officials) {
            if (o->getId() == official->getId()) {
                throw LeagueException("Official " + official->getId() + " already registered.");
            }
        }
        officials.push_back(official);
    }

    void addSpectator(Spectator* spectator) {
        if (!spectator) throw std::invalid_argument("Spectator pointer cannot be null.");
        for (const auto* s : spectators) {
            if (s->getId() == spectator->getId()) {
                throw LeagueException("Spectator " + spectator->getId() + " already registered.");
            }
        }
        spectators.push_back(spectator);
    }

    FootballTeam* findTeam(const std::string& regNo) {
        for (auto* t : teams) if (t->getRegNumber() == regNo) return t;
        return nullptr;
    }

    Stadium* findStadium(const std::string& sId) {
        for (auto* s : stadiums) if (s->getStadiumId() == sId) return s;
        return nullptr;
    }

    // schedules a match — checks both team eligibility and slot conflicts first
    Fixture& scheduleFixture(FootballTeam* home, FootballTeam* away, Stadium* stadium,
                             const std::string& date, const std::string& time) {
        if (!home || !away || !stadium) throw std::invalid_argument("Teams and stadium must be valid.");

        checkEligibility(home);
        checkEligibility(away);

        std::ostringstream oss;
        oss << "FX" << std::setw(3) << std::setfill('0') << nextFixtureNo;
        auto newFixture = std::make_unique<Fixture>(oss.str(), date, time, home, away, stadium);

        // make sure no team or stadium is double-booked for this slot
        for (const auto& fx : fixtures) {
            if (newFixture->clashesWith(*fx)) {
                throw SchedulingConflictException("Team conflict: One of the teams is already booked on "
                                                + date + " at " + time + " (" + fx->getFixtureId() + ").");
            }
            if (newFixture->sameSlot(*fx) && &fx->getStadium() == stadium) {
                throw SchedulingConflictException("Stadium conflict: " + stadium->getName()
                                                + " is already booked on " + date + " at " + time + ".");
            }
        }

        ++nextFixtureNo;
        fixtures.push_back(std::move(newFixture));
        return *fixtures.back();
    }

    // puts back a saved fixture, keeping its ID
    // team stats are saved separately, so a result is attached without counting it again
    void restoreFixture(const std::string& id, const std::string& date, const std::string& time,
                        FootballTeam* home, FootballTeam* away, Stadium* stadium,
                        const std::vector<std::string>& officialIds, const MatchResult* result) {
        if (id.size() < 3 || id.compare(0, 2, "FX") != 0) throw std::invalid_argument("bad fixture ID '" + id + "'");
        const int number = std::stoi(id.substr(2));
        auto fixture = std::make_unique<Fixture>(id, date, time, home, away, stadium);
        for (const auto& officialId : officialIds) fixture->addOfficial(&findOfficial(officialId));
        if (result) {
            fixture->setResult(*result);
            fixture->complete();
        }
        fixtures.push_back(std::move(fixture));
        nextFixtureNo = std::max(nextFixtureNo, number + 1);
    }

    // assigns a referee to a fixture — the fixture itself checks for conflicts
    void assignOfficial(const std::string& fixtureId, const std::string& officialId) {
        Fixture& fixture = findFixture(fixtureId);
        MatchOfficial& official = findOfficial(officialId);
        fixture.addOfficial(&official);
    }

    // saves the final score and updates both teams' league stats
    void recordResult(const std::string& fixtureId, int homeScore, int awayScore) {
        Fixture& fixture = findFixture(fixtureId);
        if (fixture.getStatus() == FixtureStatus::COMPLETED) {
            throw LeagueException("Fixture " + fixtureId + " is already completed.");
        }

        MatchResult res(homeScore, awayScore);
        fixture.setResult(res);
        fixture.complete();

        fixture.getHome().recordMatch(homeScore, awayScore, res.getHomePoints());
        fixture.getAway().recordMatch(awayScore, homeScore, res.getAwayPoints());
    }

    // sorts by points, then goal difference, then goals scored, then name
    std::vector<FootballTeam*> getStandings() const {
        std::vector<FootballTeam*> table = teams;
        std::sort(table.begin(), table.end(), [](const FootballTeam* a, const FootballTeam* b) {
            if (a->getPoints() != b->getPoints()) return a->getPoints() > b->getPoints();
            if (a->getGoalDifference() != b->getGoalDifference()) return a->getGoalDifference() > b->getGoalDifference();
            if (a->getGoalsFor() != b->getGoalsFor()) return a->getGoalsFor() > b->getGoalsFor();
            return a->getName() < b->getName();
        });
        return table;
    }

    void displayStandings() const {
        std::cout << "\n=======================================================\n";
        std::cout << "       " << leagueName << " - STANDINGS TABLE\n";
        std::cout << "=======================================================\n";
        std::cout << std::left << std::setw(4) << "Pos" << std::setw(20) << "Team"
                  << std::right << std::setw(4) << "P" << std::setw(4) << "W" << std::setw(4) << "D"
                  << std::setw(4) << "L" << std::setw(5) << "GF" << std::setw(5) << "GA"
                  << std::setw(5) << "GD" << std::setw(5) << "Pts" << "\n";
        std::cout << "-------------------------------------------------------\n";

        int pos = 1;
        for (const auto* t : getStandings()) {
            std::cout << std::left << std::setw(4) << pos++
                      << std::setw(20) << t->getName()
                      << std::right << std::setw(4) << t->getPlayed()
                      << std::setw(4) << t->getWins()
                      << std::setw(4) << t->getDraws()
                      << std::setw(4) << t->getLosses()
                      << std::setw(5) << t->getGoalsFor()
                      << std::setw(5) << t->getGoalsAgainst()
                      << std::setw(5) << t->getGoalDifference()
                      << std::setw(5) << t->getPoints() << "\n";
        }
        std::cout << "=======================================================\n";
    }

    void displayFixtures() const {
        std::cout << "\n================== LEAGUE FIXTURES ==================\n";
        for (const auto& fx : fixtures) {
            fx->display();
        }
        std::cout << "=====================================================\n";
    }

    // Report 1: top scorers across all teams this season
    void generateTopScorersReport() const {
        std::cout << "\n=======================================================\n";
        std::cout << "       MANAGEMENT REPORT 1: LEAGUE TOP GOAL SCORERS    \n";
        std::cout << "=======================================================\n";
        struct ScorerEntry {
            std::string playerName;
            std::string teamName;
            int goals;
            int jersey;
        };
        std::vector<ScorerEntry> scorers;

        for (const auto* team : teams) {
            for (const auto& player : team->getSquad()) {
                if (player.getGoals() > 0) {
                    scorers.push_back({player.getName(), team->getName(), player.getGoals(), player.getJerseyNumber()});
                }
            }
        }

        std::sort(scorers.begin(), scorers.end(), [](const ScorerEntry& a, const ScorerEntry& b) {
            return a.goals > b.goals;
        });

        std::cout << std::left << std::setw(4) << "Rank"
                  << std::setw(20) << "Player Name"
                  << std::setw(20) << "Team"
                  << std::right << std::setw(6) << "Jersey"
                  << std::setw(8) << "Goals" << "\n";
        std::cout << "-------------------------------------------------------\n";

        int rank = 1;
        for (const auto& s : scorers) {
            std::cout << std::left << std::setw(4) << rank++
                      << std::setw(20) << s.playerName
                      << std::setw(20) << s.teamName
                      << std::right << std::setw(6) << s.jersey
                      << std::setw(8) << s.goals << "\n";
        }
        if (scorers.empty()) {
            std::cout << "  (No goals recorded yet this season)\n";
        }
        std::cout << "=======================================================\n";
    }

    // Report 2: how full each stadium was for each fixture
    void generateAttendanceReport() const {
        std::cout << "\n=======================================================\n";
        std::cout << "    MANAGEMENT REPORT 2: STADIUM ATTENDANCE & CAPACITY \n";
        std::cout << "=======================================================\n";
        std::cout << std::left << std::setw(8) << "Fixture"
                  << std::setw(22) << "Stadium"
                  << std::right << std::setw(10) << "Capacity"
                  << std::setw(10) << "Sold"
                  << std::setw(10) << "Occupancy" << "\n";
        std::cout << "-------------------------------------------------------\n";

        for (const auto& fx : fixtures) {
            int sold = 0;
            for (const auto& t : fx->getTickets()) if (t.getIsSold()) ++sold;
            int cap = fx->getStadium().getSeatingCapacity();
            double pct = (cap > 0) ? (static_cast<double>(sold) / cap * 100.0) : 0.0;

            std::cout << std::left << std::setw(8) << fx->getFixtureId()
                      << std::setw(22) << fx->getStadium().getName().substr(0, 20)
                      << std::right << std::setw(10) << cap
                      << std::setw(10) << sold
                      << std::setw(9) << std::fixed << std::setprecision(1) << pct << "%\n";
        }
        std::cout << "=======================================================\n";
    }

    // Report 3: quick overview of each team's registration status and squad size
    void generateTeamStatusReport() const {
        std::cout << "\n=======================================================\n";
        std::cout << "    MANAGEMENT REPORT 3: TEAM REGISTRATION & SQUAD STATUS\n";
        std::cout << "=======================================================\n";
        std::cout << std::left << std::setw(8) << "ID"
                  << std::setw(20) << "Team Name"
                  << std::setw(14) << "Parish/City"
                  << std::setw(10) << "Squad"
                  << std::setw(12) << "Status" << "\n";
        std::cout << "-------------------------------------------------------\n";

        for (const auto* t : teams) {
            std::string status = t->isSuspended() ? "SUSPENDED" : (t->isRegistered() ? "ACTIVE" : "PENDING");
            std::cout << std::left << std::setw(8) << t->getRegNumber()
                      << std::setw(20) << t->getName()
                      << std::setw(14) << t->getHomeCity()
                      << std::setw(10) << t->getSquad().size()
                      << std::setw(12) << status << "\n";
        }
        std::cout << "=======================================================\n";
    }
};
