#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "exceptions.h"

using namespace std;

// Holds venue info and keeps track of which teams are using this stadium.
class Stadium {
private:
    string stadiumId;
    string name;
    string location;
    int seatingCapacity;
    string fieldCondition; ///< E.g. Excellent, Good, Fair, or Under Maintenance.
    vector<string> assignedTeamIds; ///< Registration numbers of teams using this venue.

public:
    Stadium(const string& id, const string& name, const string& location = "Kingston",
            int capacity = 25000, const string& condition = "Excellent")
        : stadiumId(id), name(name), location(location), seatingCapacity(capacity), fieldCondition(condition) {
        if (id.empty() || name.empty()) {
            throw invalid_argument("Stadium ID and Name cannot be empty");
        }
        if (capacity <= 0) {
            throw invalid_argument("Stadium capacity must be positive");
        }
        // '|' separates fields in the save file, so it can't appear inside one
        if ((id + name + location + condition).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Stadium details cannot contain '|' or line breaks.");
        }
    }

    const string& getStadiumId() const { return stadiumId; }
    const string& getName() const { return name; }
    const string& getLocation() const { return location; }
    int getSeatingCapacity() const { return seatingCapacity; }
    const string& getFieldCondition() const { return fieldCondition; }
    const vector<string>& getAssignedTeamIds() const { return assignedTeamIds; }

    void setLocation(const string& loc) { location = loc; }
    void setSeatingCapacity(int cap) {
        if (cap <= 0) throw invalid_argument("Capacity must be positive");
        seatingCapacity = cap;
    }
    void setFieldCondition(const string& cond) {
        if (cond.find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Field condition cannot contain '|' or line breaks.");
        }
        fieldCondition = cond;
    }

    void assignTeam(const string& teamId) {
        if (find(assignedTeamIds.begin(), assignedTeamIds.end(), teamId) == assignedTeamIds.end()) {
            assignedTeamIds.push_back(teamId);
        }
    }

    void displayInfo() const {
        cout << "[Stadium] " << stadiumId << " | " << name
                  << " | Loc: " << location << " | Cap: " << seatingCapacity
                  << " | Condition: " << fieldCondition
                  << " | Teams using: " << assignedTeamIds.size() << "\n";
    }
};
