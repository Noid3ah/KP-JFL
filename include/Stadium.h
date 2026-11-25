#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "exceptions.h"

// Holds venue info and keeps track of which teams are using this stadium.
class Stadium {
private:
    std::string stadiumId;
    std::string name;
    std::string location;
    int seatingCapacity;
    std::string fieldCondition; ///< E.g. Excellent, Good, Fair, or Under Maintenance.
    std::vector<std::string> assignedTeamIds; ///< Registration numbers of teams using this venue.

public:
    Stadium(const std::string& id, const std::string& name, const std::string& location = "Kingston",
            int capacity = 25000, const std::string& condition = "Excellent")
        : stadiumId(id), name(name), location(location), seatingCapacity(capacity), fieldCondition(condition) {
        if (id.empty() || name.empty()) {
            throw std::invalid_argument("Stadium ID and Name cannot be empty");
        }
        if (capacity <= 0) {
            throw std::invalid_argument("Stadium capacity must be positive");
        }
        // '|' separates fields in the save file, so it can't appear inside one
        if ((id + name + location + condition).find_first_of("|\r\n") != std::string::npos) {
            throw std::invalid_argument("Stadium details cannot contain '|' or line breaks.");
        }
    }

    const std::string& getStadiumId() const { return stadiumId; }
    const std::string& getName() const { return name; }
    const std::string& getLocation() const { return location; }
    int getSeatingCapacity() const { return seatingCapacity; }
    const std::string& getFieldCondition() const { return fieldCondition; }
    const std::vector<std::string>& getAssignedTeamIds() const { return assignedTeamIds; }

    void setLocation(const std::string& loc) { location = loc; }
    void setSeatingCapacity(int cap) {
        if (cap <= 0) throw std::invalid_argument("Capacity must be positive");
        seatingCapacity = cap;
    }
    void setFieldCondition(const std::string& cond) {
        if (cond.find_first_of("|\r\n") != std::string::npos) {
            throw std::invalid_argument("Field condition cannot contain '|' or line breaks.");
        }
        fieldCondition = cond;
    }

    void assignTeam(const std::string& teamId) {
        if (std::find(assignedTeamIds.begin(), assignedTeamIds.end(), teamId) == assignedTeamIds.end()) {
            assignedTeamIds.push_back(teamId);
        }
    }

    void displayInfo() const {
        std::cout << "[Stadium] " << stadiumId << " | " << name
                  << " | Loc: " << location << " | Cap: " << seatingCapacity
                  << " | Condition: " << fieldCondition
                  << " | Teams using: " << assignedTeamIds.size() << "\n";
    }
};
