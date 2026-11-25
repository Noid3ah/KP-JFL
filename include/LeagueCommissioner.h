#pragma once
#include <iostream>
#include "Person.h"
#include "FootballTeam.h"
#include "Fixture.h"

using namespace std;

// The commissioner handles team approvals, suspensions, and pre-match checks.
class LeagueCommissioner : public Person {
public:
    LeagueCommissioner(const string& id, const string& name, int age, const string& gender)
        : Person(id, name, age, gender) {}

    void approveTeam(FootballTeam& team) const {
        team.setRegistered(true);
        cout << "Commissioner " << getName() << " approved and registered team " << team.getName() << "\n";
    }

    void suspendTeam(FootballTeam& team, bool suspended = true) const {
        team.setSuspended(suspended);
        cout << "Commissioner " << getName() << (suspended ? " suspended " : " reinstated ")
                  << team.getName() << "\n";
    }

    bool inspectMatch(const Fixture& fixture) const {
        const bool ready = !fixture.getOfficials().empty();
        cout << "Inspection of " << fixture.getFixtureId() << " by Commissioner " << getName() << ": ";
        if (ready) {
            cout << "READY (Officials assigned)\n";
        } else {
            cout << "NOT READY (No officials assigned)\n";
        }
        return ready;
    }

    void displayInfo() const override {
        cout << "[LeagueCommissioner] ID: " << id << " | Name: " << name
                  << " | Age: " << age << " | Gender: " << gender << "\n";
    }
};
