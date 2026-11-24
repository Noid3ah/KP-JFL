#pragma once
#include <string>

// TEMPORARY STUB - Person 2 owns FootballTeam.
// The league module relies on: isRegistered, isSuspended, getName and recordMatch.
class FootballTeam {
private:
    std::string regNumber, name;
    bool registered = false, suspended = false;
    int played = 0, wins = 0, draws = 0, losses = 0;
    int goalsFor = 0, goalsAgainst = 0, points = 0;
public:
    FootballTeam(const std::string& regNumber, const std::string& name)
        : regNumber(regNumber), name(name) {}

    const std::string& getRegNumber() const { return regNumber; }
    const std::string& getName() const { return name; }
    bool isRegistered() const { return registered; }
    bool isSuspended() const { return suspended; }
    void setRegistered(bool r) { registered = r; }
    void setSuspended(bool s) { suspended = s; }

    int getPlayed() const { return played; }
    int getWins() const { return wins; }
    int getDraws() const { return draws; }
    int getLosses() const { return losses; }
    int getGoalsFor() const { return goalsFor; }
    int getGoalsAgainst() const { return goalsAgainst; }
    int getGoalDifference() const { return goalsFor - goalsAgainst; }
    int getPoints() const { return points; }

    void recordMatch(int gf, int ga, int pts) {
        ++played; goalsFor += gf; goalsAgainst += ga; points += pts;
        if (gf > ga) ++wins; else if (gf == ga) ++draws; else ++losses;
    }
};
