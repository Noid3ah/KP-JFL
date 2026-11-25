#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Person.h"

using namespace std;

// A registered player on a club squad.
// League rules: must be at least 16 years old, jersey number between 1 and 99.
class Player : public Person {
private:
    string position;      // Forward, Midfielder, Defender, Goalkeeper
    int jerseyNumber;          // 1 - 99
    int goals;                 // Career league goals scored
    string playingStatus; // Active, Injured, Benched, Suspended
    string teamId;        // Reference to parent club

public:
    Player(const string& id, const string& name, int age, const string& gender,
           const string& position, int jerseyNumber, const string& teamId = "")
        : Person(id, name, age, gender),
          position(position),
          jerseyNumber(jerseyNumber),
          goals(0),
          playingStatus("Active"),
          teamId(teamId) {
        // '|' separates fields in the save file, so it can't appear inside one
        if ((id + name + gender + position).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Player details cannot contain '|' or line breaks.");
        }
        // league rule: players must be 16 or older
        if (age < 16) {
            throw InvalidAgeException("Player " + name + " must be at least 16 years old (Entered: " + to_string(age) + ").");
        }
        if (jerseyNumber < 1 || jerseyNumber > 99) {
            throw invalid_argument("Jersey number must be between 1 and 99.");
        }
    }

    // getters
    const string& getPosition() const { return position; }
    int getJerseyNumber() const { return jerseyNumber; }
    int getGoals() const { return goals; }
    const string& getPlayingStatus() const { return playingStatus; }
    const string& getTeamId() const { return teamId; }

    // setters
    void setPosition(const string& pos) { position = pos; }
    void setPlayingStatus(const string& status) { playingStatus = status; }
    void setTeamId(const string& tid) { teamId = tid; }

    void addGoals(int g) {
        if (g < 0) throw invalid_argument("Goals cannot be negative.");
        goals += g;
    }

    void displayInfo() const override {
        cout << "  [Player] ID: " << id << " | " << name
                  << " | Age: " << age << " | Pos: " << position
                  << " | #" << jerseyNumber << " | Goals: " << goals
                  << " | Status: " << playingStatus << "\n";
    }
};
