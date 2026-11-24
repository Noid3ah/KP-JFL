#pragma once
#include <iostream>
#include "Person.h"
#include "FootballTeam.h"
#include "Fixture.h"

class LeagueCommissioner : public Person {
public:
    LeagueCommissioner(const std::string& id, const std::string& name, int age, const std::string& gender)
        : Person(id, name, age, gender) {
    }

    void approveTeam(FootballTeam& team) const {
        team.setRegistered(true);
        std::cout << "Commissioner " << getName() << " approved " << team.getName() << "\n";
    }

    bool inspectMatch(const Fixture& fixture) const {
        const bool ready = !fixture.getOfficials().empty();

        std::cout << "Inspection of " << fixture.getFixtureId() << ": ";
        if (ready) {
            std::cout << "READY";
        } else {
            std::cout << "NOT READY (no officials)";
        }
        std::cout << "\n";

        return ready;
    }

    void displayInfo() const override {
        std::cout << getId() << " | " << getName() << " | League Commissioner\n";
    }
};
