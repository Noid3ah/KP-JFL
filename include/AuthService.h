#pragma once
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

/// Role-based access level used to route a signed-in user to the appropriate menu.
enum class UserRole {
    ADMIN_ALL,               // Full System Admin
    LEAGUE_COMMISSIONER,     // Person 1: League Ops & Fixtures
    TEAM_MANAGER,            // Person 2: Team & Personnel Manager
    STADIUM_COORDINATOR,     // Person 3: Stadium Operations & Ticketing
    SHOP_MANAGER             // Person 4: Stadium Shop, Finance & Data
};

/// Credentials and display metadata for one application account.
struct UserAccount {
    string username;
    string password;
    UserRole    role;
    string displayName;
};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers: convert between UserRole and its string tag stored in the file
// ─────────────────────────────────────────────────────────────────────────────
inline string roleToString(UserRole r) {
    switch (r) {
        case UserRole::ADMIN_ALL:            return "ADMIN_ALL";
        case UserRole::LEAGUE_COMMISSIONER:  return "LEAGUE_COMMISSIONER";
        case UserRole::TEAM_MANAGER:         return "TEAM_MANAGER";
        case UserRole::STADIUM_COORDINATOR:  return "STADIUM_COORDINATOR";
        case UserRole::SHOP_MANAGER:         return "SHOP_MANAGER";
        default:                             return "SHOP_MANAGER";
    }
}

inline UserRole roleFromString(const string& s) {
    if (s == "ADMIN_ALL")           return UserRole::ADMIN_ALL;
    if (s == "LEAGUE_COMMISSIONER") return UserRole::LEAGUE_COMMISSIONER;
    if (s == "TEAM_MANAGER")        return UserRole::TEAM_MANAGER;
    if (s == "STADIUM_COORDINATOR") return UserRole::STADIUM_COORDINATOR;
    return UserRole::SHOP_MANAGER;
}

/**
 * Provides role-based login authentication with flat-file persistence.
 *
 * On construction the service loads accounts from kpfl_users.txt (if it
 * exists) and merges them with the built-in demo accounts, so credentials
 * are never lost between program restarts.  Every call to saveUsers() writes
 * the current account table back to disk in a human-readable pipe-delimited
 * format that is easy to inspect or edit in any text editor.
 *
 * File format (one account per line):
 *   username|password|ROLE_TAG|Display Name
 *
 * Example:
 *   admin|admin123|ADMIN_ALL|League System Administrator
 *   commish|pass123|LEAGUE_COMMISSIONER|Marcus Brown (Commissioner)
 */
class AuthService {
private:
    unordered_map<string, UserAccount> users;
    string usersFile = "kpfl_users.txt";

    // ── Seed the built-in demo accounts ──────────────────────────────────────
    void seedDefaults() {
        auto add = [&](const string& u, const string& p,
                       UserRole r, const string& name) {
            if (users.find(u) == users.end())   // don't overwrite file version
                users[u] = {u, p, r, name};
        };
        add("admin",   "admin123", UserRole::ADMIN_ALL,           "League System Administrator");
        add("commish", "pass123",  UserRole::LEAGUE_COMMISSIONER, "Marcus Brown (Commissioner)");
        add("manager", "pass123",  UserRole::TEAM_MANAGER,        "Ludlow Bernard (Team Manager)");
        add("stadium", "pass123",  UserRole::STADIUM_COORDINATOR, "Stadium & Ticketing Lead");
        add("shop",    "pass123",  UserRole::SHOP_MANAGER,        "Stadium Shop & Finance Lead");
    }

public:
    // ── Construction: load file first, then fill any missing defaults ─────────
    AuthService() {
        loadUsers();   // load persisted accounts (may be empty on first run)
        seedDefaults();// add built-in accounts that are not already in the file
        saveUsers();   // write the merged set back so the file is always current
    }

    // ── Persist: write every account to kpfl_users.txt ───────────────────────
    void saveUsers() const {
        ofstream file(usersFile);
        if (!file.is_open()) return;   // silently skip if path is unwritable

        file << "# KPFL Login Accounts — format: username|password|role|displayName\n";
        file << "# DO NOT delete or reorder these lines manually.\n";
        for (const auto& [key, acc] : users) {
            file << acc.username    << "|"
                 << acc.password    << "|"
                 << roleToString(acc.role) << "|"
                 << acc.displayName << "\n";
        }
        file.close();
    }

    // ── Load: read kpfl_users.txt and populate the in-memory table ───────────
    void loadUsers() {
        ifstream file(usersFile);
        if (!file.is_open()) return;   // first run — file doesn't exist yet

        string line;
        while (getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;   // skip comments

            stringstream ss(line);
            string uname, pwd, roleStr, name;
            if (!getline(ss, uname,   '|')) continue;
            if (!getline(ss, pwd,     '|')) continue;
            if (!getline(ss, roleStr, '|')) continue;
            if (!getline(ss, name))         continue;

            users[uname] = {uname, pwd, roleFromString(roleStr), name};
        }
        file.close();
    }

    // ── Authenticate: return a pointer to the account, or nullptr ────────────
    const UserAccount* login(const string& username,
                             const string& password) const {
        auto it = users.find(username);
        if (it != users.end() && it->second.password == password)
            return &(it->second);
        return nullptr;
    }

    // ── Display: print the account table to the console ──────────────────────
    void displayAvailableAccounts() const {
        cout << "\n======================================================\n";
        cout << "                 AVAILABLE DEMO LOGINS               \n";
        cout << "======================================================\n";
        cout << "  Role                     Username    Password       \n";
        cout << "------------------------------------------------------\n";
        cout << "  1. Full Administrator    admin       admin123       \n";
        cout << "  2. League Ops Lead       commish     pass123        \n";
        cout << "  3. Team Manager          manager     pass123        \n";
        cout << "  4. Stadium Coordinator   stadium     pass123        \n";
        cout << "  5. Stadium Shop & Data   shop        pass123        \n";
        cout << "------------------------------------------------------\n";
        cout << "  Credentials are saved in: kpfl_users.txt           \n";
        cout << "======================================================\n\n";
    }
};
