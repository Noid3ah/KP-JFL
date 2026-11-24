#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "Person.h"

class MatchOfficial : public Person {
private:
    std::string role;
    std::string rank;
    bool available;
    std::vector<std::string> bookedSlots;

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

    bool isFree(const std::string& date, const std::string& time) const {
        if (!available) {
            return false;
        }

        const std::string requestedSlot = slotKey(date, time);
        const auto bookedSlot = std::find(bookedSlots.begin(), bookedSlots.end(), requestedSlot);
        return bookedSlot == bookedSlots.end();
    }

    void assignSlot(const std::string& date, const std::string& time) {
        if (date.empty() || time.empty()) {
            throw std::invalid_argument("A booked slot needs a date and time");
        }
        if (!isFree(date, time)) {
            throw std::invalid_argument("Match official is not free on " + date + " at " + time);
        }
        bookedSlots.push_back(slotKey(date, time));
    }

    void displayInfo() const override {
        std::cout << getId() << " | " << getName() << " | " << role << " (" << rank << ")"
                  << " | ";

        if (available) {
            std::cout << "Available";
        } else {
            std::cout << "Unavailable";
        }

        std::cout << "\n";
    }
};
