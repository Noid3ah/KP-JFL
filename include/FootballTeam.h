#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "Player.h"
#include "Coach.h"
#include "exceptions.h"

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
    std::string regNumber;
    std::string name;
    std::string homeCity;
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
    std::vector<Player> squad;
    std::vector<Coach> coachingStaff;

public:
    // reg number and name are both required — throws if either is blank
    FootballTeam(const std::string& regNumber, const std::string& name, const std::string& city = "Kingston")
        : regNumber(regNumber), name(name), homeCity(city) {
        if (regNumber.empty() || name.empty()) {
            throw std::invalid_argument("Team registration number and name cannot be empty");
        }
    }

    const std::string& getRegNumber() const { return regNumber; }
    const std::string& getName() const { return name; }
    const std::string& getHomeCity() const { return homeCity; }
    bool isRegistered() const { return registered; }
    bool isSuspended() const { return suspended; }
    void setRegistered(bool r) { registered = r; }
    void setSuspended(bool s) { suspended = s; }
    void setHomeCity(const std::string& city) { homeCity = city; }
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

    // adds a player copy to the squad
    // throws if the player ID or jersey number is already taken
    void addPlayer(const Player& player) {
        for (const auto& p : squad) {
            if (p.getId() == player.getId()) {
                throw DuplicatePersonException("Player with ID " + player.getId() + " already in squad");
            }
            if (p.getJerseyNumber() == player.getJerseyNumber()) {
                throw PersonnelException("Jersey #" + std::to_string(player.getJerseyNumber()) + " is already taken by " + p.getName());
            }
        }
        Player copy = player;
        copy.setTeamId(regNumber);
        squad.push_back(copy);
    }

    // removes a player by ID, returns false if no match found
    bool removePlayer(const std::string& playerId) {
        auto it = std::find_if(squad.begin(), squad.end(), [&](const Player& p) {
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
    bool updatePlayer(const std::string& playerId, const std::string& name, int age,
                      const std::string& position, int jerseyNumber, const std::string& status) {
        Player* existing = findPlayer(playerId);
        if (!existing) return false;

        for (const auto& p : squad) {
            if (p.getId() != playerId && p.getJerseyNumber() == jerseyNumber) {
                throw PersonnelException("Jersey #" + std::to_string(jerseyNumber) + " is already taken by " + p.getName());
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
    Player* findPlayer(const std::string& playerId) {
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

    // removes a coach by ID, returns false if not found
    bool removeCoach(const std::string& coachId) {
        auto it = std::find_if(coachingStaff.begin(), coachingStaff.end(), [&](const Coach& c) {
            return c.getId() == coachId;
        });
        if (it != coachingStaff.end()) {
            coachingStaff.erase(it);
            return true;
        }
        return false;
    }

    const std::vector<Player>& getSquad() const { return squad; }
    std::vector<Player>& getSquad() { return squad; }
    const std::vector<Coach>& getCoachingStaff() const { return coachingStaff; }
    std::vector<Coach>& getCoachingStaff() { return coachingStaff; }

    // prints team info, record, coaches and full squad to the console
    void displayTeamInfo() const {
        std::cout << "\n========================================\n";
        std::cout << "Team: " << name << " (" << regNumber << ") - City: " << homeCity << "\n";
        std::cout << "Status: " << (registered ? "Registered" : "Unregistered")
                  << (suspended ? " [SUSPENDED]" : " [ACTIVE]") << "\n";
        std::cout << "Record: Played" << played << " Wins" << wins << " Draws" << draws << " Losses" << losses
                  << " | GF: " << goalsFor << " GA: " << goalsAgainst
                  << " GD: " << getGoalDifference() << " Pts: " << points << "\n";

        std::cout << "-- Coaches (" << coachingStaff.size() << ") --\n";
        for (const auto& c : coachingStaff) {
            c.displayInfo();
        }

        std::cout << "-- Squad (" << squad.size() << " players) --\n";
        for (const auto& p : squad) {
            p.displayInfo();
        }
        std::cout << "========================================\n";
    }
};
