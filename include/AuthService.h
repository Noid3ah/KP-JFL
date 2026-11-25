#pragma once
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include "SafeFile.h"

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
    if (s == "SHOP_MANAGER")        return UserRole::SHOP_MANAGER;
    throw AuthException("unknown role '" + s + "'");
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
    string usersFile;

    static string toLower(string text) {
        for (char& ch : text) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        return text;
    }

    // ── Seed the built-in demo accounts ──────────────────────────────────────
    bool seedDefaults() {
        const size_t before = users.size();
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
        return users.size() != before;
    }

public:
    // ── Construction: load file first, then fill any missing defaults ─────────
    explicit AuthService(const string& file = "kpfl_users.txt") : usersFile(file) {
        loadUsers();                     // load persisted accounts (may be none on first run)
        if (seedDefaults()) saveUsers(); // add built-in accounts that are not already in the file
    }

    // ── Persist: write every account to kpfl_users.txt ───────────────────────
    void saveUsers() const {
        ostringstream file;
        file << "# KPFL Login Accounts — format: username|password|role|displayName\n";
        file << "# DO NOT delete or reorder these lines manually.\n";
        for (const auto& [key, acc] : users) {
            file << acc.username    << "|"
                 << acc.password    << "|"
                 << roleToString(acc.role) << "|"
                 << acc.displayName << "\n";
        }
        writeFileSafely(file.str(), usersFile);
    }

    // ── Load: read kpfl_users.txt and populate the in-memory table ───────────
    // a damaged line stops startup, so it can't be lost at the next save
    void loadUsers() {
        ifstream file(usersFile);
        if (!file.is_open()) return;   // first run — file doesn't exist yet

        string line;
        for (int lineNo = 1; getline(file, line); ++lineNo) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;   // skip comments

            try {
                stringstream ss(line);
                string uname, pwd, roleStr, name;
                if (!getline(ss, uname, '|') || !getline(ss, pwd, '|') || !getline(ss, roleStr, '|') || !getline(ss, name)
                    || uname.empty() || pwd.empty()) {
                    throw AuthException("expected username|password|role|displayName");
                }
                if (users.count(uname)) throw AuthException("duplicate username '" + uname + "'");
                users[uname] = {uname, pwd, roleFromString(roleStr), name};
            } catch (const exception& e) {
                throw PersistenceException(usersFile + " line " + to_string(lineNo) + ": " + e.what());
            }
        }
    }

    // ── Rules: each returns an error message, or "" if the text is acceptable ─
    string usernameError(const string& username) const {
        if (username.size() < 3 || username.size() > 20) return "Use 3 to 20 characters.";
        for (unsigned char ch : username) {
            const bool ok = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')
                         || ch == '.' || ch == '_' || ch == '-';
            if (!ok) return "Use letters, numbers, dots, hyphens and underscores only.";
        }
        if (toLower(username) == "cancel") return "That username is not available.";
        for (const auto& [key, acc] : users) {
            if (toLower(key) == toLower(username)) return "Username '" + username + "' is already taken. Choose another.";
        }
        return "";
    }

    static string passwordError(const string& password) {
        if (password.size() < 6 || password.size() > 40) return "Use 6 to 40 characters.";
        if (password.find_first_of("| \t\r\n") != string::npos) return "Spaces and '|' are not allowed.";
        if (toLower(password) == "cancel") return "Choose a different password.";
        return "";
    }

    // ── Register: add a staff account; administrators can't be registered ────
    void registerUser(const string& username, const string& password, const string& displayName, UserRole role) {
        if (role == UserRole::ADMIN_ALL) throw AuthException("Administrator accounts cannot be registered.");
        string error = usernameError(username);
        if (error.empty()) error = passwordError(password);
        if (error.empty() && (displayName.empty() || displayName.find_first_of("|\r\n") != string::npos)) {
            error = "Enter a display name without '|' or line breaks.";
        }
        if (!error.empty()) throw AuthException(error);

        users[username] = {username, password, role, displayName};
        try {
            saveUsers();
        } catch (...) {
            users.erase(username); // not saved, so it must not work either
            throw;
        }
    }

    // ── Change password: the current one must be given ───────────────────────
    void changePassword(const string& username, const string& current, const string& replacement) {
        if (!login(username, current)) throw AuthException("Username or current password is incorrect.");
        const string error = passwordError(replacement);
        if (!error.empty()) throw AuthException(error);
        if (replacement == current) throw AuthException("The new password must be different.");

        users[username].password = replacement;
        try {
            saveUsers();
        } catch (...) {
            users[username].password = current;
            throw;
        }
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
        cout << "                 DEFAULT DEMO LOGINS                \n";
        cout << "======================================================\n";
        cout << "  Role                     Username    Password       \n";
        cout << "------------------------------------------------------\n";
        cout << "  1. Full Administrator    admin       admin123       \n";
        cout << "  2. League Ops Lead       commish     pass123        \n";
        cout << "  3. Team Manager          manager     pass123        \n";
        cout << "  4. Stadium Coordinator   stadium     pass123        \n";
        cout << "  5. Stadium Shop & Data   shop        pass123        \n";
        cout << "------------------------------------------------------\n";
        cout << "  These are the starting passwords; they can be changed\n";
        cout << "======================================================\n\n";
    }
};
