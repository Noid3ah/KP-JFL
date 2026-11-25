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

using namespace std;

// The main league class — ties together teams, stadiums, officials, fixtures, and standings.
// Owns fixture objects but only holds references to everything else.
class FootballLeague {
private:
    string leagueName;
    LeagueCommissioner* commissioner = nullptr;
    vector<FootballTeam*> teams;
    vector<Stadium*> stadiums;
    vector<MatchOfficial*> officials;
    vector<Spectator*> spectators;
    vector<unique_ptr<Fixture>> fixtures;
    int nextFixtureNo = 1;

    // looks up a fixture — throws if the ID doesn't match anything
    Fixture& findFixture(const string& id) {
        for (auto& f : fixtures) {
            if (f->getFixtureId() == id) return *f;
        }
        throw LeagueException("Fixture not found with ID: " + id);
    }

    // looks up an official — throws if not found
    MatchOfficial& findOfficial(const string& id) {
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
    explicit FootballLeague(const string& name) : leagueName(name) {
        if (name.empty()) throw invalid_argument("League name cannot be empty.");
    }

    // getters
    const string& getLeagueName() const { return leagueName; }
    LeagueCommissioner* getCommissioner() const { return commissioner; }
    const vector<FootballTeam*>& getTeams() const { return teams; }
    const vector<Stadium*>& getStadiums() const { return stadiums; }
    const vector<MatchOfficial*>& getOfficials() const { return officials; }
    const vector<Spectator*>& getSpectators() const { return spectators; }
    const vector<unique_ptr<Fixture>>& getFixtures() const { return fixtures; }

    void setCommissioner(LeagueCommissioner* comm) {
        if (!comm) throw invalid_argument("Commissioner pointer cannot be null.");
        commissioner = comm;
    }

    void addTeam(FootballTeam* team) {
        if (!team) throw invalid_argument("Team pointer cannot be null.");
        for (const auto* existing : teams) {
            if (existing->getRegNumber() == team->getRegNumber()) {
                throw LeagueException("Team " + team->getRegNumber() + " is already in the league.");
            }
        }
        teams.push_back(team);
    }

    void addStadium(Stadium* stadium) {
        if (!stadium) throw invalid_argument("Stadium pointer cannot be null.");
        for (const auto* s : stadiums) {
            if (s->getStadiumId() == stadium->getStadiumId()) {
                throw LeagueException("Stadium " + stadium->getStadiumId() + " already exists.");
            }
        }
        stadiums.push_back(stadium);
    }

    void addOfficial(MatchOfficial* official) {
        if (!official) throw invalid_argument("Official pointer cannot be null.");
        for (const auto* o : officials) {
            if (o->getId() == official->getId()) {
                throw LeagueException("Official " + official->getId() + " already registered.");
            }
        }
        officials.push_back(official);
    }

    void addSpectator(Spectator* spectator) {
        if (!spectator) throw invalid_argument("Spectator pointer cannot be null.");
        for (const auto* s : spectators) {
            if (s->getId() == spectator->getId()) {
                throw LeagueException("Spectator " + spectator->getId() + " already registered.");
            }
        }
        spectators.push_back(spectator);
    }

    FootballTeam* findTeam(const string& regNo) {
        for (auto* t : teams) if (t->getRegNumber() == regNo) return t;
        return nullptr;
    }

    Stadium* findStadium(const string& sId) {
        for (auto* s : stadiums) if (s->getStadiumId() == sId) return s;
        return nullptr;
    }

    // explains why a slot can't be used, or returns "" if it is free
    string slotConflict(const FootballTeam* home, const FootballTeam* away, const Stadium* stadium,
                             const string& date, const string& time) const {
        for (const auto& fx : fixtures) {
            if (fx->getDate() != date || fx->getTime() != time) continue;
            if (fx->involvesTeam(home) || fx->involvesTeam(away)) {
                return "Team conflict: One of the teams is already booked on "
                     + date + " at " + time + " (" + fx->getFixtureId() + ").";
            }
            if (&fx->getStadium() == stadium) {
                return "Stadium conflict: " + stadium->getName() + " is already booked on " + date + " at " + time + ".";
            }
        }
        return "";
    }

    // schedules a match — checks both team eligibility and slot conflicts first
    Fixture& scheduleFixture(FootballTeam* home, FootballTeam* away, Stadium* stadium,
                             const string& date, const string& time) {
        if (!home || !away || !stadium) throw invalid_argument("Teams and stadium must be valid.");

        checkEligibility(home);
        checkEligibility(away);

        ostringstream oss;
        oss << "FX" << setw(3) << setfill('0') << nextFixtureNo;
        auto newFixture = make_unique<Fixture>(oss.str(), date, time, home, away, stadium);

        // make sure no team or stadium is double-booked for this slot
        const string conflict = slotConflict(home, away, stadium, date, time);
        if (!conflict.empty()) throw SchedulingConflictException(conflict);

        ++nextFixtureNo;
        fixtures.push_back(std::move(newFixture));
        return *fixtures.back();
    }

    // puts back a saved fixture, keeping its ID
    // team stats are saved separately, so a result is attached without counting it again
    void restoreFixture(const string& id, const string& date, const string& time,
                        FootballTeam* home, FootballTeam* away, Stadium* stadium,
                        const vector<string>& officialIds, const MatchResult* result) {
        if (id.size() < 3 || id.compare(0, 2, "FX") != 0) throw invalid_argument("bad fixture ID '" + id + "'");
        const int number = stoi(id.substr(2));
        auto fixture = make_unique<Fixture>(id, date, time, home, away, stadium);
        for (const auto& officialId : officialIds) fixture->addOfficial(&findOfficial(officialId));
        if (result) {
            fixture->setResult(*result);
            fixture->complete();
        }
        fixtures.push_back(std::move(fixture));
        nextFixtureNo = max(nextFixtureNo, number + 1);
    }

    // assigns a referee to a fixture — the fixture itself checks for conflicts
    void assignOfficial(const string& fixtureId, const string& officialId) {
        Fixture& fixture = findFixture(fixtureId);
        MatchOfficial& official = findOfficial(officialId);
        fixture.addOfficial(&official);
    }

    // saves the final score and updates both teams' league stats
    void recordResult(const string& fixtureId, int homeScore, int awayScore) {
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
    vector<FootballTeam*> getStandings() const {
        vector<FootballTeam*> table = teams;
        sort(table.begin(), table.end(), [](const FootballTeam* a, const FootballTeam* b) {
            if (a->getPoints() != b->getPoints()) return a->getPoints() > b->getPoints();
            if (a->getGoalDifference() != b->getGoalDifference()) return a->getGoalDifference() > b->getGoalDifference();
            if (a->getGoalsFor() != b->getGoalsFor()) return a->getGoalsFor() > b->getGoalsFor();
            return a->getName() < b->getName();
        });
        return table;
    }

    void displayStandings() const {
        cout << "\n=======================================================\n";
        cout << "       " << leagueName << " - STANDINGS TABLE\n";
        cout << "=======================================================\n";
        cout << left << setw(4) << "Pos" << setw(20) << "Team"
                  << right << setw(4) << "P" << setw(4) << "W" << setw(4) << "D"
                  << setw(4) << "L" << setw(5) << "GF" << setw(5) << "GA"
                  << setw(5) << "GD" << setw(5) << "Pts" << "\n";
        cout << "-------------------------------------------------------\n";

        int pos = 1;
        for (const auto* t : getStandings()) {
            cout << left << setw(4) << pos++
                      << setw(20) << t->getName()
                      << right << setw(4) << t->getPlayed()
                      << setw(4) << t->getWins()
                      << setw(4) << t->getDraws()
                      << setw(4) << t->getLosses()
                      << setw(5) << t->getGoalsFor()
                      << setw(5) << t->getGoalsAgainst()
                      << setw(5) << t->getGoalDifference()
                      << setw(5) << t->getPoints() << "\n";
        }
        cout << "=======================================================\n";
    }

    void displayFixtures() const {
        cout << "\n================== LEAGUE FIXTURES ==================\n";
        for (const auto& fx : fixtures) {
            fx->display();
        }
        cout << "=====================================================\n";
    }

    // Report 1: top scorers across all teams this season
    void generateTopScorersReport() const {
        cout << "\n=======================================================\n";
        cout << "       MANAGEMENT REPORT 1: LEAGUE TOP GOAL SCORERS    \n";
        cout << "=======================================================\n";
        struct ScorerEntry {
            string playerName;
            string teamName;
            int goals;
            int jersey;
        };
        vector<ScorerEntry> scorers;

        for (const auto* team : teams) {
            for (const auto& player : team->getSquad()) {
                if (player.getGoals() > 0) {
                    scorers.push_back({player.getName(), team->getName(), player.getGoals(), player.getJerseyNumber()});
                }
            }
        }

        sort(scorers.begin(), scorers.end(), [](const ScorerEntry& a, const ScorerEntry& b) {
            return a.goals > b.goals;
        });

        cout << left << setw(4) << "Rank"
                  << setw(20) << "Player Name"
                  << setw(20) << "Team"
                  << right << setw(6) << "Jersey"
                  << setw(8) << "Goals" << "\n";
        cout << "-------------------------------------------------------\n";

        int rank = 1;
        for (const auto& s : scorers) {
            cout << left << setw(4) << rank++
                      << setw(20) << s.playerName
                      << setw(20) << s.teamName
                      << right << setw(6) << s.jersey
                      << setw(8) << s.goals << "\n";
        }
        if (scorers.empty()) {
            cout << "  (No goals recorded yet this season)\n";
        }
        cout << "=======================================================\n";
    }

    // Report 2: how full each stadium was for each fixture
    void generateAttendanceReport() const {
        cout << "\n=======================================================\n";
        cout << "    MANAGEMENT REPORT 2: STADIUM ATTENDANCE & CAPACITY \n";
        cout << "=======================================================\n";
        cout << left << setw(8) << "Fixture"
                  << setw(22) << "Stadium"
                  << right << setw(10) << "Capacity"
                  << setw(10) << "Sold"
                  << setw(10) << "Occupancy" << "\n";
        cout << "-------------------------------------------------------\n";

        for (const auto& fx : fixtures) {
            int sold = 0;
            for (const auto& t : fx->getTickets()) if (t.getIsSold()) ++sold;
            int cap = fx->getStadium().getSeatingCapacity();
            double pct = (cap > 0) ? (static_cast<double>(sold) / cap * 100.0) : 0.0;

            cout << left << setw(8) << fx->getFixtureId()
                      << setw(22) << fx->getStadium().getName().substr(0, 20)
                      << right << setw(10) << cap
                      << setw(10) << sold
                      << setw(9) << fixed << setprecision(1) << pct << "%\n";
        }
        cout << "=======================================================\n";
    }

    // Report 3: quick overview of each team's registration status and squad size
    void generateTeamStatusReport() const {
        cout << "\n=======================================================\n";
        cout << "    MANAGEMENT REPORT 3: TEAM REGISTRATION & SQUAD STATUS\n";
        cout << "=======================================================\n";
        cout << left << setw(8) << "ID"
                  << setw(20) << "Team Name"
                  << setw(14) << "Parish/City"
                  << setw(10) << "Squad"
                  << setw(12) << "Status" << "\n";
        cout << "-------------------------------------------------------\n";

        for (const auto* t : teams) {
            string status = t->isSuspended() ? "SUSPENDED" : (t->isRegistered() ? "ACTIVE" : "PENDING");
            cout << left << setw(8) << t->getRegNumber()
                      << setw(20) << t->getName()
                      << setw(14) << t->getHomeCity()
                      << setw(10) << t->getSquad().size()
                      << setw(12) << status << "\n";
        }
        cout << "=======================================================\n";
    }
};
