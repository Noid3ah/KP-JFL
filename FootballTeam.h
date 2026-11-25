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
    /**
     * Creates a team. The registration number and name are required.
     *
     * @throws std::invalid_argument if regNumber or name is empty.
     */
    FootballTeam(const std::string& regNumber, const std::string& name, const std::string& city = "Kingston")
        : regNumber(regNumber), name(name), homeCity(city) {
        if (regNumber.empty() || name.empty()) {
            throw std::invalid_argument("Team registration number and name cannot be empty");
        }
    }

    /// Returns the immutable league registration number.
    const std::string& getRegNumber() const { return regNumber; }

    /// Returns the club name.
    const std::string& getName() const { return name; }

    /// Returns the city's name used as the team's home location.
    const std::string& getHomeCity() const { return homeCity; }

    /// Reports whether the team has completed league registration.
    bool isRegistered() const { return registered; }

    /// Reports whether the team is currently suspended from competition.
    bool isSuspended() const { return suspended; }

    /// Changes the team's registration state.
    void setRegistered(bool r) { registered = r; }

    /// Changes the team's suspension state.
    void setSuspended(bool s) { suspended = s; }

    /// Updates the team's home city.
    void setHomeCity(const std::string& city) { homeCity = city; }

    /// Returns the number of matches recorded for this team.
    int getPlayed() const { return played; }

    /// Returns the number of matches won.
    int getWins() const { return wins; }

    /// Returns the number of drawn matches.
    int getDraws() const { return draws; }

    /// Returns the number of matches lost.
    int getLosses() const { return losses; }

    /// Returns the number of goals scored in all recorded matches.
    int getGoalsFor() const { return goalsFor; }

    /// Returns the number of goals conceded in all recorded matches.
    int getGoalsAgainst() const { return goalsAgainst; }

    /// Returns goals scored minus goals conceded.
    int getGoalDifference() const { return goalsFor - goalsAgainst; }

    /// Returns the cumulative league points.
    int getPoints() const { return points; }

    /**
     * Updates all cumulative match statistics for one completed match.
     *
     * The win, draw, and loss counters are selected from gf and ga. Points
     * are supplied separately because league or match rules decide their
     * value outside this class.
     */
    void recordMatch(int gf, int ga, int pts) {
        ++played;
        goalsFor += gf;
        goalsAgainst += ga;
        points += pts;
        if (gf > ga) ++wins;
        else if (gf == ga) ++draws;
        else ++losses;
    }

    /**
     * Adds a copy of a player to the squad.
     *
     * Player IDs must be unique within the squad, and jersey numbers must
     * not already be assigned. The copy is associated with this team's
     * registration number before it is stored.
     *
     * @throws DuplicatePersonException if the player ID already exists.
     * @throws PersonnelException if the jersey number is already in use.
     */
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

    /**
     * Removes a player by ID.
     *
     * @return true if a matching player was removed; false if it was absent.
     */
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

    /**
     * Finds a player by ID.
     *
     * @return A pointer to the stored player, or nullptr if no match exists.
     *         The returned pointer refers to a squad element and must not be
     *         retained across operations that may remove or add personnel.
     */
    Player* findPlayer(const std::string& playerId) {
        for (auto& p : squad) {
            if (p.getId() == playerId) return &p;
        }
        return nullptr;
    }

    /**
     * Adds a copy of a coach to the coaching staff.
     *
     * Coach IDs must be unique. The copy is associated with this team's
     * registration number before it is stored.
     *
     * @throws DuplicatePersonException if the coach ID already exists.
     */
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

    /**
     * Removes a coach by ID.
     *
     * @return true if a matching coach was removed; false if it was absent.
     */
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

    /// Returns read-only access to the squad.
    const std::vector<Player>& getSquad() const { return squad; }

    /// Returns mutable access to the squad.
    std::vector<Player>& getSquad() { return squad; }

    /// Returns read-only access to the coaching staff.
    const std::vector<Coach>& getCoachingStaff() const { return coachingStaff; }

    /// Returns mutable access to the coaching staff.
    std::vector<Coach>& getCoachingStaff() { return coachingStaff; }

    /// Prints the team summary, standings statistics, coaches, and squad to std::cout.
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
