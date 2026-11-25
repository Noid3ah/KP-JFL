#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Person.h"

/**
 * =========================================================================
 * CLASS: Coach (Person 2 - Subclass of Person)
 * =========================================================================
 * PURPOSE:
 *  Represents a certified football coach or manager for a club.
 *
 * BUSINESS RULES ENFORCED:
 *  - Minimum age requirement: Must be at least 21 years old.
 * =========================================================================
 */
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
        // Enforce coach age rule: Minimum 21 years old
        if (age < 21) {
            throw InvalidAgeException("Coach " + name + " must be at least 21 years old (Entered: " + std::to_string(age) + ").");
        }
    }

    const std::string& getQualification() const { return qualification; }
    const std::string& getCoachingRole() const { return coachingRole; }
    int getYearsExperience() const { return yearsExperience; }
    const std::string& getTeamId() const { return teamId; }

    void setTeamId(const std::string& tid) { teamId = tid; }

    // Polymorphic display implementation
    void displayInfo() const override {
        std::cout << "  [Coach]  ID: " << id << " | " << name
                  << " | Role: " << coachingRole << " | Qual: " << qualification
                  << " | Exp: " << yearsExperience << " yrs\n";
    }
};
