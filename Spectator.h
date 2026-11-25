#pragma once
#include <iostream>
#include <string>
#include "Person.h"

/**
 * Represents a match attendee and stores the spectator's contact and fan details.
 */
class Spectator : public Person {
private:
    std::string email;
    std::string phone;
    std::string preferredTeam; ///< Empty means the spectator has no preferred team.
    int ticketsBought;         ///< Number of tickets recorded as purchased for this spectator.

public:
    Spectator(const std::string& id, const std::string& name, int age, const std::string& gender,
              const std::string& email = "", const std::string& phone = "",
              const std::string& preferredTeam = "")
        : Person(id, name, age, gender),
          email(email),
          phone(phone),
          preferredTeam(preferredTeam),
          ticketsBought(0) {}

    const std::string& getEmail() const { return email; }
    const std::string& getPhone() const { return phone; }
    const std::string& getPreferredTeam() const { return preferredTeam; }
    int getTicketsBought() const { return ticketsBought; }

    void setEmail(const std::string& mail) { email = mail; }
    void setPhone(const std::string& ph) { phone = ph; }
    void setPreferredTeam(const std::string& team) { preferredTeam = team; }

    void incrementTickets(int count = 1) {
        if (count > 0) ticketsBought += count;
    }

    void displayInfo() const override {
        std::cout << "[Spectator] ID: " << id << " | Name: " << name
                  << " | Age: " << age << " | Fan of: "
                  << (preferredTeam.empty() ? "Neutral" : preferredTeam)
                  << " | Tickets: " << ticketsBought << "\n";
    }
};
