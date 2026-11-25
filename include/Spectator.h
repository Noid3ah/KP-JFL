#pragma once
#include <iostream>
#include <string>
#include "Person.h"

using namespace std;

// A fan attending a match — stores contact info and their preferred team.
class Spectator : public Person {
private:
    string email;
    string phone;
    string preferredTeam; ///< Empty means the spectator has no preferred team.
    int ticketsBought;         ///< Number of tickets recorded as purchased for this spectator.

public:
    Spectator(const string& id, const string& name, int age, const string& gender,
              const string& email = "", const string& phone = "",
              const string& preferredTeam = "")
        : Person(id, name, age, gender),
          email(email),
          phone(phone),
          preferredTeam(preferredTeam),
          ticketsBought(0) {
        // '|' separates fields in the save file, so it can't appear inside one
        if ((id + name + gender + email + phone + preferredTeam).find_first_of("|\r\n") != string::npos) {
            throw invalid_argument("Spectator details cannot contain '|' or line breaks.");
        }
    }

    const string& getEmail() const { return email; }
    const string& getPhone() const { return phone; }
    const string& getPreferredTeam() const { return preferredTeam; }
    int getTicketsBought() const { return ticketsBought; }

    void setEmail(const string& mail) { email = mail; }
    void setPhone(const string& ph) { phone = ph; }
    void setPreferredTeam(const string& team) { preferredTeam = team; }

    void incrementTickets(int count = 1) {
        if (count > 0) ticketsBought += count;
    }

    // used when saved tickets are loaded and counted again
    void resetTickets() { ticketsBought = 0; }

    void displayInfo() const override {
        cout << "[Spectator] ID: " << id << " | Name: " << name
                  << " | Age: " << age << " | Fan of: "
                  << (preferredTeam.empty() ? "Neutral" : preferredTeam)
                  << " | Tickets: " << ticketsBought << "\n";
    }
};
