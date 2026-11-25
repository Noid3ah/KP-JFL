#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "Player.h"
#include "Coach.h"
#include "exceptions.h"

using namespace std;

/**
 * Represents one registered football club in the league.
 *
 * A team owns a list of player copies and coach copies. When personnel are
 * added, this class updates their team ID to match the team's registration
 * number. League statistics are cumulative totals for matches already
 * recorded through recordMatch().
 */
class FootballTeam {
private:
    // Identity and league status.
    string regNumber;
    string name;
    string homeCity;
    bool registered = false;
    bool suspended = false;

    // Cumulative match and table statistics.
    int played = 0;
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int goalsFor = 0;
    int goalsAgainst = 0;
    int points = 0;

    // Personnel belonging to this club.
    vector<Player> squad;
    vector<Coach> coachingStaff;

public:
    // reg number and name are both required — throws if either is blank
    FootballTeam(const string& regNumber, const string& name, const string& city = "Kingston")
        : regNumber(regNumber), name(name), homeCity(city) {
        if (regNumber.empty() || name.empty()) {
            throw invalid_argument("Team registration number and name cannot be empty");
        }
        // '|' separates fields in the save file, so it can't appear inside one
        if ((regNumber + name + city).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Team details cannot contain '|' or line breaks.");
        }
    }

    const string& getRegNumber() const { return regNumber; }
    const string& getName() const { return name; }
    const string& getHomeCity() const { return homeCity; }
    bool isRegistered() const { return registered; }
    bool isSuspended() const { return suspended; }
    void setRegistered(bool r) { registered = r; }
    void setSuspended(bool s) { suspended = s; }
    void setHomeCity(const string& city) { homeCity = city; }
    int getPlayed() const { return played; }
    int getWins() const { return wins; }
    int getDraws() const { return draws; }
    int getLosses() const { return losses; }
    int getGoalsFor() const { return goalsFor; }
    int getGoalsAgainst() const { return goalsAgainst; }
    int getGoalDifference() const { return goalsFor - goalsAgainst; }
    int getPoints() const { return points; }

    // called after a result is recorded — updates the team's stats for the table
    // points come from outside since the league decides how many to award
    void recordMatch(int gf, int ga, int pts) {
        ++played;
        goalsFor += gf;
        goalsAgainst += ga;
        points += pts;
        if (gf > ga) ++wins;
        else if (gf == ga) ++draws;
        else ++losses;
    }

    // restores a saved league record
    void setRecord(int p, int w, int d, int l, int gf, int ga, int pts) {
        if (w < 0 || d < 0 || l < 0 || gf < 0 || ga < 0 || pts < 0 || p != w + d + l) {
            throw invalid_argument("Invalid league record for team " + name + ".");
        }
        played = p; wins = w; draws = d; losses = l;
        goalsFor = gf; goalsAgainst = ga; points = pts;
    }

    // adds a player copy to the squad
    // throws if the player ID or jersey number is already taken
    void addPlayer(const Player& player) {
        for (const auto& p : squad) {
            if (p.getId() == player.getId()) {
                throw DuplicatePersonException("Player with ID " + player.getId() + " already in squad");
            }
            if (p.getJerseyNumber() == player.getJerseyNumber()) {
                throw PersonnelException("Jersey #" + to_string(player.getJerseyNumber()) + " is already taken by " + p.getName());
            }
        }
        Player copy = player;
        copy.setTeamId(regNumber);
        squad.push_back(copy);
    }

    // removes a player by ID, returns false if no match found
    bool removePlayer(const string& playerId) {
        auto it = find_if(squad.begin(), squad.end(), [&](const Player& p) {
            return p.getId() == playerId;
        });
        if (it != squad.end()) {
            squad.erase(it);
            return true;
        }
        return false;
    }

    // updates a player's details; ID, gender, team and goals are kept
    // everything is validated before anything changes, returns false if no match found
    bool updatePlayer(const string& playerId, const string& name, int age,
                      const string& position, int jerseyNumber, const string& status) {
        Player* existing = findPlayer(playerId);
        if (!existing) return false;

        for (const auto& p : squad) {
            if (p.getId() != playerId && p.getJerseyNumber() == jerseyNumber) {
                throw PersonnelException("Jersey #" + to_string(jerseyNumber) + " is already taken by " + p.getName());
            }
        }
        if (status != "Active" && status != "Injured" && status != "Benched" && status != "Suspended") {
            throw PersonnelException("Invalid status '" + status + "'. Use Active, Injured, Benched or Suspended.");
        }

        // rebuilding through the constructor reuses its name, age and jersey range rules
        Player updated(playerId, name, age, existing->getGender(), position, jerseyNumber, regNumber);
        updated.addGoals(existing->getGoals());
        updated.setPlayingStatus(status);
        *existing = updated;
        return true;
    }

    // looks up a player by ID — returns nullptr if not found
    // don't hold onto this pointer after modifying the squad
    Player* findPlayer(const string& playerId) {
        for (auto& p : squad) {
            if (p.getId() == playerId) return &p;
        }
        return nullptr;
    }

    // adds a coach to the staff — throws if that coach ID is already there
    void addCoach(const Coach& coach) {
        for (const auto& c : coachingStaff) {
            if (c.getId() == coach.getId()) {
                throw DuplicatePersonException("Coach with ID " + coach.getId() + " already on coaching staff");
            }
        }
        Coach copy = coach;
        copy.setTeamId(regNumber);
        coachingStaff.push_back(copy);
    }

    // updates a coach's details; ID, gender and team are kept
    // everything is validated before anything changes, returns false if no match found
    bool updateCoach(const string& coachId, const string& name, int age,
                     const string& qualification, const string& role, int yearsExperience) {
        for (auto& c : coachingStaff) {
            if (c.getId() != coachId) continue;
            // rebuilding through the constructor reuses its name and age rules
            c = Coach(coachId, name, age, c.getGender(), qualification, role, yearsExperience, regNumber);
            return true;
        }
        return false;
    }

    // removes a coach by ID, returns false if not found
    bool removeCoach(const string& coachId) {
        auto it = find_if(coachingStaff.begin(), coachingStaff.end(), [&](const Coach& c) {
            return c.getId() == coachId;
        });
        if (it != coachingStaff.end()) {
            coachingStaff.erase(it);
            return true;
        }
        return false;
    }

    const vector<Player>& getSquad() const { return squad; }
    vector<Player>& getSquad() { return squad; }
    const vector<Coach>& getCoachingStaff() const { return coachingStaff; }
    vector<Coach>& getCoachingStaff() { return coachingStaff; }

    // prints team info, record, coaches and full squad to the console
    void displayTeamInfo() const {
        cout << "\n========================================\n";
        cout << "Team: " << name << " (" << regNumber << ") - City: " << homeCity << "\n";
        cout << "Status: " << (registered ? "Registered" : "Unregistered")
                  << (suspended ? " [SUSPENDED]" : " [ACTIVE]") << "\n";
        cout << "Record: Played" << played << " Wins" << wins << " Draws" << draws << " Losses" << losses
                  << " | GF: " << goalsFor << " GA: " << goalsAgainst
                  << " GD: " << getGoalDifference() << " Pts: " << points << "\n";

        cout << "-- Coaches (" << coachingStaff.size() << ") --\n";
        for (const auto& c : coachingStaff) {
            c.displayInfo();
        }

        cout << "-- Squad (" << squad.size() << " players) --\n";
        for (const auto& p : squad) {
            p.displayInfo();
        }
        cout << "========================================\n";
    }
};
