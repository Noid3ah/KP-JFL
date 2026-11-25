#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Person.h"

// A coaching staff member attached to a club.
// Must be at least 21 — that's a league rule enforced in the constructor.
class Coach : public Person {
private:
    std::string qualification;  // e.g., "CONCACAF Pro License", "UEFA A License"
    std::string coachingRole;   // e.g., "Head Coach", "Assistant Coach"
    int yearsExperience;
    std::string teamId;

public:
    Coach(const std::string& id, const std::string& name, int age, const std::string& gender,
          const std::string& qualification, const std::string& role, int yearsExperience = 1,
          const std::string& teamId = "")
        : Person(id, name, age, gender),
          qualification(qualification),
          coachingRole(role),
          yearsExperience(yearsExperience),
          teamId(teamId) {
        // '|' separates fields in the save file, so it can't appear inside one
        if ((id + name + gender + qualification + role).find_first_of("|\r\n") != std::string::npos) {
            throw std::invalid_argument("Coach details cannot contain '|' or line breaks.");
        }
        // coaches have to be at least 21
        if (age < 21) {
            throw InvalidAgeException("Coach " + name + " must be at least 21 years old (Entered: " + std::to_string(age) + ").");
        }
    }

    const std::string& getQualification() const { return qualification; }
    const std::string& getCoachingRole() const { return coachingRole; }
    int getYearsExperience() const { return yearsExperience; }
    const std::string& getTeamId() const { return teamId; }

    void setTeamId(const std::string& tid) { teamId = tid; }

    void displayInfo() const override {
        std::cout << "  [Coach]  ID: " << id << " | " << name
                  << " | Role: " << coachingRole << " | Qual: " << qualification
                  << " | Exp: " << yearsExperience << " yrs\n";
    }
};
