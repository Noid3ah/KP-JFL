#pragma once
#include <iostream>
#include "Person.h"
#include "FootballTeam.h"
#include "Fixture.h"

/**
 * Represents the league authority responsible for team registration,
 * suspensions, and basic match-readiness inspections.
 */
class LeagueCommissioner : public Person {
public:
    LeagueCommissioner(const std::string& id, const std::string& name, int age, const std::string& gender)
        : Person(id, name, age, gender) {}

    void approveTeam(FootballTeam& team) const {
        team.setRegistered(true);
        std::cout << "Commissioner " << getName() << " approved and registered team " << team.getName() << "\n";
    }

    void suspendTeam(FootballTeam& team, bool suspended = true) const {
        team.setSuspended(suspended);
        std::cout << "Commissioner " << getName() << (suspended ? " suspended " : " reinstated ")
                  << team.getName() << "\n";
    }

    bool inspectMatch(const Fixture& fixture) const {
        const bool ready = !fixture.getOfficials().empty();
        std::cout << "Inspection of " << fixture.getFixtureId() << " by Commissioner " << getName() << ": ";
        if (ready) {
            std::cout << "READY (Officials assigned)\n";
        } else {
            std::cout << "NOT READY (No officials assigned)\n";
        }
        return ready;
    }

    void displayInfo() const override {
        std::cout << "[LeagueCommissioner] ID: " << id << " | Name: " << name
                  << " | Age: " << age << " | Gender: " << gender << "\n";
    }
};
