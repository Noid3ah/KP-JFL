#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "Person.h"
#include "exceptions.h"

/**
 * Represents a referee or other match official and tracks date/time assignments.
 */
class MatchOfficial : public Person {
private:
    std::string role; // Referee, Assistant Referee, Fourth Official, VAR
    std::string rank; // Grade A, Grade B, FIFA Badge
    bool available; ///< Independent availability flag; unavailable officials cannot accept slots.
    std::vector<std::string> bookedSlots; ///< Confirmed assignments in "date time" format.

    static std::string slotKey(const std::string& date, const std::string& time) {
        return date + " " + time;
    }

public:
    MatchOfficial(const std::string& id, const std::string& name, int age,
                  const std::string& gender, const std::string& role, const std::string& rank)
        : Person(id, name, age, gender),
          role(role),
          rank(rank),
          available(true) {}

    const std::string& getRole() const { return role; }
    const std::string& getRank() const { return rank; }
    bool isAvailable() const { return available; }
    void setAvailable(bool a) { available = a; }
    void setRole(const std::string& r) { role = r; }
    void setRank(const std::string& rk) { rank = rk; }

    const std::vector<std::string>& getBookedSlots() const { return bookedSlots; }

    /// Returns false when globally unavailable or already booked for the requested date and time.
    bool isFree(const std::string& date, const std::string& time) const {
        if (!available) return false;
        const std::string requestedSlot = slotKey(date, time);
        return std::find(bookedSlots.begin(), bookedSlots.end(), requestedSlot) == bookedSlots.end();
    }

    void assignSlot(const std::string& date, const std::string& time) {
        if (date.empty() || time.empty()) {
            throw std::invalid_argument("A booked slot needs a date and time");
        }
        if (!isFree(date, time)) {
            throw OfficialConflictException("Match official " + name + " is not free on " + date + " at " + time);
        }
        bookedSlots.push_back(slotKey(date, time));
    }

    void displayInfo() const override {
        std::cout << "[MatchOfficial] ID: " << id << " | Name: " << name
                  << " | Role: " << role << " | Rank: " << rank
                  << " | Available: " << (available ? "Yes" : "No") << "\n";
    }
};
