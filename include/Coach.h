#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Person.h"

using namespace std;

// A coaching staff member attached to a club.
// Must be at least 21 — that's a league rule enforced in the constructor.
class Coach : public Person {
private:
    string qualification;  // e.g., "CONCACAF Pro License", "UEFA A License"
    string coachingRole;   // e.g., "Head Coach", "Assistant Coach"
    int yearsExperience;
    string teamId;

public:
    Coach(const string& id, const string& name, int age, const string& gender,
          const string& qualification, const string& role, int yearsExperience = 1,
          const string& teamId = "")
        : Person(id, name, age, gender),
          qualification(qualification),
          coachingRole(role),
          yearsExperience(yearsExperience),
          teamId(teamId) {
        // '|' separates fields in the save file, so it can't appear inside one
        if ((id + name + gender + qualification + role).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Coach details cannot contain '|' or line breaks.");
        }
        // coaches have to be at least 21
        if (age < 21) {
            throw InvalidAgeException("Coach " + name + " must be at least 21 years old (Entered: " + to_string(age) + ").");
        }
    }

    const string& getQualification() const { return qualification; }
    const string& getCoachingRole() const { return coachingRole; }
    int getYearsExperience() const { return yearsExperience; }
    const string& getTeamId() const { return teamId; }

    void setTeamId(const string& tid) { teamId = tid; }

    void displayInfo() const override {
        cout << "  [Coach]  ID: " << id << " | " << name
                  << " | Role: " << coachingRole << " | Qual: " << qualification
                  << " | Exp: " << yearsExperience << " yrs\n";
    }
};
