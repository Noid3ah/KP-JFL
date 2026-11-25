#pragma once
#include <string>
#include <unordered_map>
#include <iostream>

/// Role-based access level used to route a signed-in user to the appropriate menu.
enum class UserRole {
    ADMIN_ALL,               // Full System Admin
    LEAGUE_COMMISSIONER,     // Person 1: League Ops & Fixtures
    TEAM_MANAGER,            // Person 2: Team & Personnel Manager
    STADIUM_COORDINATOR,     // Person 3: Stadium Operations & Ticketing
    SHOP_MANAGER             // Person 4: Stadium Shop, Finance & Data
};

/// Credentials and display metadata for one demo or application account.
struct UserAccount {
    std::string username;
    std::string password;
    UserRole role;
    std::string displayName;
};

/**
 * Provides in-memory demonstration login accounts and credential validation.
 *
 * This is not a secure password service: demo passwords are stored directly
 * in the account table and are intended for local training use.
 */
class AuthService {
private:
    std::unordered_map<std::string, UserAccount> users;

public:
    AuthService() {
        // Pre-configured accounts for each role and admin
        users["admin"] = {"admin", "admin123", UserRole::ADMIN_ALL, "League System Administrator"};
        users["commish"] = {"commish", "pass123", UserRole::LEAGUE_COMMISSIONER, "Marcus Brown (Commissioner)"};
        users["manager"] = {"manager", "pass123", UserRole::TEAM_MANAGER, "Ludlow Bernard (Team Manager)"};
        users["stadium"] = {"stadium", "pass123", UserRole::STADIUM_COORDINATOR, "Stadium & Ticketing Lead"};
        users["shop"] = {"shop", "pass123", UserRole::SHOP_MANAGER, "Stadium Shop & Finance Lead"};
    }

    const UserAccount* login(const std::string& username, const std::string& password) const {
        auto it = users.find(username);
        if (it != users.end() && it->second.password == password) {
            return &(it->second);
        }
        return nullptr;
    }

    void displayAvailableAccounts() const {
        std::cout << "\n======================================================\n";
        std::cout << "                 AVAILABLE DEMO LOGINS                \n";
        std::cout << "======================================================\n";
        std::cout << "  Role                   Username     Password        \n";
        std::cout << "------------------------------------------------------\n";
        std::cout << "  1. Full Administrator  admin        admin123        \n";
        std::cout << "  2. League Ops Lead     commish      pass123         \n";
        std::cout << "  3. Team Manager        manager      pass123         \n";
        std::cout << "  4. Stadium Coordinator stadium      pass123         \n";
        std::cout << "  5. Stadium Shop & Data shop         pass123         \n";
        std::cout << "======================================================\n\n";
    }
};
