#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "Person.h"
#include "exceptions.h"

using namespace std;

// A match official (referee, AR, etc.) — keeps track of which slots they're already booked for.
class MatchOfficial : public Person {
private:
    string role; // Referee, Assistant Referee, Fourth Official, VAR
    string rank; // Grade A, Grade B, FIFA Badge
    bool available; ///< Independent availability flag; unavailable officials cannot accept slots.
    vector<string> bookedSlots; ///< Confirmed assignments in "date time" format.

    static string slotKey(const string& date, const string& time) {
        return date + " " + time;
    }

public:
    MatchOfficial(const string& id, const string& name, int age,
                  const string& gender, const string& role, const string& rank)
        : Person(id, name, age, gender),
          role(role),
          rank(rank),
          available(true) {}

    const string& getRole() const { return role; }
    const string& getRank() const { return rank; }
    bool isAvailable() const { return available; }
    void setAvailable(bool a) { available = a; }
    void setRole(const string& r) { role = r; }
    void setRank(const string& rk) { rank = rk; }

    const vector<string>& getBookedSlots() const { return bookedSlots; }

    // false if they're marked unavailable or already have that slot booked
    bool isFree(const string& date, const string& time) const {
        if (!available) return false;
        const string requestedSlot = slotKey(date, time);
        return find(bookedSlots.begin(), bookedSlots.end(), requestedSlot) == bookedSlots.end();
    }

    void assignSlot(const string& date, const string& time) {
        if (date.empty() || time.empty()) {
            throw invalid_argument("A booked slot needs a date and time");
        }
        if (!isFree(date, time)) {
            throw OfficialConflictException("Match official " + name + " is not free on " + date + " at " + time);
        }
        bookedSlots.push_back(slotKey(date, time));
    }

    void displayInfo() const override {
        cout << "[MatchOfficial] ID: " << id << " | Name: " << name
                  << " | Role: " << role << " | Rank: " << rank
                  << " | Available: " << (available ? "Yes" : "No") << "\n";
    }
};
