#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include "exceptions.h"
#include "Spectator.h"

/// Available seating categories for a match ticket.
enum class TicketCategory {
    BLEACHERS,
    GRANDSTAND,
    VIP,
    ULTRA_VIP
};

inline std::string ticketCategoryToString(TicketCategory cat) {
    switch (cat) {
        case TicketCategory::BLEACHERS: return "Bleachers";
        case TicketCategory::GRANDSTAND: return "Grandstand";
        case TicketCategory::VIP: return "VIP";
        case TicketCategory::ULTRA_VIP: return "Ultra VIP";
        default: return "General";
    }
}

inline double getDefaultCategoryPrice(TicketCategory cat) {
    switch (cat) {
        case TicketCategory::BLEACHERS: return 1500.0; // JMD
        case TicketCategory::GRANDSTAND: return 3500.0;
        case TicketCategory::VIP: return 7000.0;
        case TicketCategory::ULTRA_VIP: return 12000.0;
        default: return 2000.0;
    }
}

// One seat reservation for a match. Starts as available until sellTo() is called.
// Trying to sell an already-sold ticket will throw.
class Ticket {
private:
    std::string ticketId;
    std::string fixtureId;
    std::string stadiumId;
    std::string seatNumber;
    TicketCategory category;
    double price;
    std::string spectatorId;
    bool isSold = false;

public:
    Ticket(const std::string& ticketId, const std::string& fixtureId, const std::string& stadiumId,
           const std::string& seatNumber, TicketCategory category, double price = 0.0)
        : ticketId(ticketId), fixtureId(fixtureId), stadiumId(stadiumId),
          seatNumber(seatNumber), category(category),
          price(price <= 0.0 ? getDefaultCategoryPrice(category) : price) {
        if (ticketId.empty() || fixtureId.empty() || stadiumId.empty()) {
            throw std::invalid_argument("Ticket requires ID, fixture ID, and stadium ID");
        }
    }

    const std::string& getTicketId() const { return ticketId; }
    const std::string& getFixtureId() const { return fixtureId; }
    const std::string& getStadiumId() const { return stadiumId; }
    const std::string& getSeatNumber() const { return seatNumber; }
    TicketCategory getCategory() const { return category; }
    double getPrice() const { return price; }
    const std::string& getSpectatorId() const { return spectatorId; }
    bool getIsSold() const { return isSold; }

    void sellTo(Spectator& spectator) {
        if (isSold) {
            throw StadiumTicketingException("Ticket " + ticketId + " has already been sold");
        }
        spectatorId = spectator.getId();
        isSold = true;
        spectator.incrementTickets(1);
    }

    void displayInfo() const {
        std::cout << "Ticket #" << ticketId << " | Fixture: " << fixtureId
                  << " | Seat: " << seatNumber << " | Cat: " << ticketCategoryToString(category)
                  << " | Price: $" << std::fixed << std::setprecision(2) << price
                  << " | " << (isSold ? ("Sold to " + spectatorId) : "Available") << "\n";
    }
};
