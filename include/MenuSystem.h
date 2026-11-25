#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <functional>
#include "AuthService.h"
#include "FootballLeague.h"
#include "LeagueCommissioner.h"
#include "FootballTeam.h"
#include "Stadium.h"
#include "StadiumShop.h"
#include "DataPersistence.h"

using namespace std;

/**
 * Console front end for login and role-specific application operations.
 *
 * MenuSystem coordinates references to the league, commissioner, and shop,
 * but does not own those objects. It routes one logged-in user to the menu
 * associated with their UserRole.
 */
class MenuSystem {
private:
    struct InputCancelled {}; // thrown when the user enters 0 at a numbered list

    FootballLeague& league;
    LeagueCommissioner& commissioner;
    StadiumShop& shop;
    AuthService auth;
    const UserAccount* currentUser = nullptr;

    void clearScreen() const {
#if defined(_WIN32) || defined(_WIN64)
        system("cls");
#else
        system("clear");
#endif
    }

    void pausePrompt() const {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    static string toLower(string text) {
        for (char& ch : text) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        return text;
    }

    // reads one trimmed line; typing 'cancel' backs out of the form
    string readLine() {
        string in;
        if (!getline(cin, in)) throw InputCancelled{};
        const auto first = in.find_first_not_of(" \t\r");
        in = first == string::npos ? "" : in.substr(first, in.find_last_not_of(" \t\r") - first + 1);
        if (toLower(in) == "cancel") throw InputCancelled{};
        return in;
    }

    // keeps asking until check() returns no error message
    // if a current value is given, pressing Enter keeps it
    template <typename Check>
    string askUntil(const string& label, const string& current, Check check) {
        while (true) {
            cout << label << (current.empty() ? "" : " [" + current + "]") << ": ";
            string in = readLine();
            if (in.empty() && !current.empty()) return current;
            const string error = check(in);
            if (error.empty()) return in;
            cout << "[ERROR] " << error << "\n";
        }
    }

    string askText(const string& label, const string& current = "") {
        return askUntil(label, current, [](const string& in) {
            return string(in.empty() ? "This cannot be blank." : in.find('|') != string::npos ? "'|' is not allowed." : "");
        });
    }

    string askOptional(const string& label) {
        return askUntil(label, "", [](const string& in) {
            return string(in.find('|') != string::npos ? "'|' is not allowed." : "");
        });
    }

    // asks for an ID that none of the given items use yet; upper and lower case count as the same
    template <typename Items, typename Describe>
    string askNewId(const string& label, const Items& items, Describe describe) {
        return askUntil(label, "", [&](const string& in) -> string {
            if (in.empty() || in.find_first_of(" \t|") != string::npos) return "Enter an ID without spaces or '|'.";
            for (const auto& item : items) {
                const pair<string, string> idAndName = describe(item);
                if (toLower(idAndName.first) == toLower(in)) {
                    return in + " already belongs to " + idAndName.second + ". Choose another.";
                }
            }
            return "";
        });
    }

    // extraCheck, if given, returns an error message for a number that is in range but still not allowed
    int askInt(const string& label, int min, int max, const string& current = "",
               const function<string(int)>& extraCheck = nullptr) {
        const string text = askUntil(label, current, [&](const string& in) {
            istringstream ss(in);
            int v;
            const bool whole = (ss >> v) && (ss >> ws).eof();
            if (!whole || v < min || v > max) {
                return "Enter a whole number from " + to_string(min) + " to " + to_string(max) + ".";
            }
            return extraCheck ? extraCheck(v) : string();
        });
        return stoi(text);
    }

    double askMoney(const string& label) {
        const string text = askUntil(label, "", [](const string& in) {
            const bool ok = !in.empty() && in.size() <= 12 && in.find_first_not_of("0123456789.") == string::npos
                && in != "." && in.find('.') == in.rfind('.');
            return string(ok ? "" : "Enter an amount such as 8500 or 8500.50.");
        });
        return stod(text);
    }

    static string money(double amount) {
        ostringstream out;
        out << "JMD $" << fixed << setprecision(2) << amount;
        return out.str();
    }

    int askJersey(const FootballTeam& team, const string& label, const string& playerId, const string& current = "") {
        return askInt(label, 1, 99, current, [&](int number) -> string {
            for (const auto& p : team.getSquad()) {
                if (p.getId() != playerId && p.getJerseyNumber() == number) {
                    return "#" + to_string(number) + " is already taken by " + p.getName() + ". Choose another.";
                }
            }
            return "";
        });
    }

    string askDate(const string& label) {
        return askUntil(label, "", [](const string& in) {
            return string(isValidDate(in) ? "" : "Enter a real date as YYYY-MM-DD, e.g. 2026-12-05.");
        });
    }

    string askTime(const string& label) {
        return askUntil(label, "", [](const string& in) {
            return string(isValidTime(in) ? "" : "Enter a 24-hour time as HH:MM, e.g. 15:00.");
        });
    }

    // shows a numbered list and returns the chosen index, asking again until the number is valid
    // if a current value is given, pressing Enter keeps it and -1 is returned
    int pick(const string& title, const vector<string>& options, const string& current = "") {
        if (options.empty()) throw runtime_error(title + ": nothing to choose from yet.");

        cout << "\n" << title << (current.empty() ? "" : " [" + current + "]") << ":\n";
        for (size_t i = 0; i < options.size(); ++i) {
            cout << "  " << i + 1 << ". " << options[i] << "\n";
        }
        while (true) {
            cout << "Enter number (1-" << options.size() << ", 0 to cancel"
                      << (current.empty() ? "" : ", Enter to keep") << "): ";
            const string in = readLine();
            if (in.empty() && !current.empty()) return -1;
            if (!in.empty() && in.size() <= 4 && in.find_first_not_of("0123456789") == string::npos) {
                const int n = stoi(in);
                if (n == 0) throw InputCancelled{};
                if (n <= static_cast<int>(options.size())) return n - 1;
            }
            cout << "[ERROR] Please choose a number from the list.\n";
        }
    }

    // same, but returns the chosen text
    string pickText(const string& title, const vector<string>& options, const string& current = "") {
        const int i = pick(title, options, current);
        return i < 0 ? current : options[i];
    }

    // same, with a last option for typing something not on the list
    string pickOrType(const string& title, vector<string> options, const string& current = "") {
        options.push_back("Other (type it in)");
        string choice = pickText(title, options, current);
        if (choice == options.back()) {
            choice = askText("Enter " + title);
        }
        return choice;
    }

    // picks one item from a collection, using label() to describe each
    template <typename Items, typename Label>
    auto& pickFrom(const string& title, Items& items, Label label) {
        vector<string> options;
        for (const auto& item : items) options.push_back(label(item));
        return items[pick(title, options)];
    }

    FootballTeam* pickTeam(const string& title = "Team", const FootballTeam* exclude = nullptr) {
        vector<FootballTeam*> teams;
        for (auto* t : league.getTeams()) if (t != exclude) teams.push_back(t);
        return pickFrom(title, teams, [](const FootballTeam* t) {
            return "[" + t->getRegNumber() + "] " + t->getName() + (t->isSuspended() ? " (SUSPENDED)" : "");
        });
    }

    // only accepts a team that is allowed to play
    FootballTeam* pickEligibleTeam(const string& title, const FootballTeam* exclude = nullptr) {
        while (true) {
            FootballTeam* team = pickTeam(title, exclude);
            if (team->isRegistered() && !team->isSuspended()) return team;
            cout << "[ERROR] " << team->getName() << " is " << (team->isSuspended() ? "suspended" : "not registered")
                      << " and cannot be scheduled.\n";
        }
    }

    // only accepts a fixture that has not been played
    Fixture& pickOpenFixture() {
        while (true) {
            Fixture& fx = pickFixture();
            if (fx.getStatus() != FixtureStatus::COMPLETED) return fx;
            cout << "[ERROR] " << fx.getFixtureId() << " is already completed. Choose another.\n";
        }
    }

    Stadium* pickStadium() {
        return pickFrom("Stadium", league.getStadiums(), [](const Stadium* st) {
            return "[" + st->getStadiumId() + "] " + st->getName() + " (Capacity: "
                 + to_string(st->getSeatingCapacity()) + ", Condition: " + st->getFieldCondition() + ")";
        });
    }

    Fixture& pickFixture() {
        return *pickFrom("Fixture", league.getFixtures(), [](const unique_ptr<Fixture>& fx) {
            return "[" + fx->getFixtureId() + "] " + fx->getHome().getName() + " vs " + fx->getAway().getName()
                 + " (" + fx->getDate() + " " + fx->getTime() + " @ " + fx->getStadium().getName() + ", "
                 + (fx->getStatus() == FixtureStatus::COMPLETED ? "Completed" : "Scheduled")
                 + ", Officials: " + to_string(fx->getOfficials().size())
                 + ", Tickets: " + to_string(fx->getTickets().size()) + ")";
        });
    }

    Player& pickPlayer(FootballTeam& team) {
        return pickFrom("Player", team.getSquad(), [](const Player& p) {
            return "[" + p.getId() + "] #" + to_string(p.getJerseyNumber()) + " " + p.getName()
                 + " (" + p.getPosition() + ", " + p.getPlayingStatus() + ")";
        });
    }

    Coach& pickCoach(FootballTeam& team) {
        return pickFrom("Coach", team.getCoachingStaff(), [](const Coach& c) {
            return "[" + c.getId() + "] " + c.getName() + " (" + c.getCoachingRole() + ", "
                 + c.getQualification() + ", " + to_string(c.getYearsExperience()) + " yrs)";
        });
    }

    string pickGender() { return pick("Gender", {"Male", "Female"}) == 0 ? "M" : "F"; }

    string pickPosition(const string& current = "") {
        return pickText("Position", {"Forward", "Midfielder", "Defender", "Goalkeeper"}, current);
    }

    string pickQualification(const string& current = "") {
        return pickOrType("Qualification", {"CONCACAF Pro", "CONCACAF A", "CONCACAF B", "FIFA Elite License",
                                            "UEFA A License"}, current);
    }

    string pickCoachRole(const string& current = "") {
        return pickOrType("Role", {"Head Coach", "Assistant Coach", "Goalkeeping Coach", "Fitness Coach"}, current);
    }

    string pickCondition() {
        return pickText("Field Condition", {"Excellent", "Good", "Fair", "Maintenance"});
    }

    string pickParish(const string& title) {
        return pickOrType(title, {"Kingston", "St. Andrew", "St. Catherine", "Clarendon", "Manchester", "St. Elizabeth",
                                  "Westmoreland", "Hanover", "St. James", "Trelawny", "St. Ann", "St. Mary",
                                  "Portland", "St. Thomas"});
    }

public:
    MenuSystem(FootballLeague& league, LeagueCommissioner& comm, StadiumShop& shop)
        : league(league), commissioner(comm), shop(shop) {}

    void run() {
        while (true) {
            if (!currentUser) {
                if (!showLoginPage()) {
                    clearScreen();
                    cout << "\n======================================================\n";
                    cout << "  Exiting Jamaican Football League Management System. \n";
                    cout << "                    Goodbye!                          \n";
                    cout << "======================================================\n\n";
                    break;
                }
            } else {
                routeMenu();
            }
        }
    }

    bool showLoginPage() {
        clearScreen();
        cout << "======================================================\n";
        cout << "      KINGSTON PREMIER FOOTBALL LEAGUE (KPFL)        \n";
        cout << "                  SECURE LOGIN PORTAL                 \n";
        cout << "======================================================\n";
        auth.displayAvailableAccounts();

        cout << "Options:\n";
        cout << "  1. Log In\n";
        cout << "  2. Exit Application\n";
        cout << "Select option (1-2): ";

        int choice;
        if (!(cin >> choice)) {
            clearInput();
            return false;
        }
        clearInput();

        if (choice == 2) return false;

        string username, password;
        try {
            cout << "\nType 'cancel' to go back.\n";
            username = askText("Enter Username");
            password = askText("Enter Password");
        } catch (const InputCancelled&) {
            return true; // back to the login page
        }

        currentUser = auth.login(username, password);
        if (currentUser) {
            clearScreen();
            cout << "\n======================================================\n";
            cout << " [LOGIN SUCCESSFUL] Welcome, " << currentUser->displayName << "!\n";
            cout << "======================================================\n";
            pausePrompt();
            return true;
        } else {
            cout << "\n[ERROR] Invalid credentials! Please check your username and password.\n";
            pausePrompt();
            return true;
        }
    }

    void routeMenu() {
        switch (currentUser->role) {
            case UserRole::ADMIN_ALL:
                showAdminMenu();
                break;
            case UserRole::LEAGUE_COMMISSIONER:
                showLeagueMenu();
                break;
            case UserRole::TEAM_MANAGER:
                showTeamPersonnelMenu();
                break;
            case UserRole::STADIUM_COORDINATOR:
                showStadiumTicketingMenu();
                break;
            case UserRole::SHOP_MANAGER:
                showShopFinanceMenu();
                break;
        }
    }

    // ==========================================
    // Master Admin Menu (Access to all 4 modules)
    // ==========================================
    void showAdminMenu() {
        while (currentUser && currentUser->role == UserRole::ADMIN_ALL) {
            clearScreen();
            cout << "======================================================\n";
            cout << "           SYSTEM ADMINISTRATOR MASTER DASHBOARD       \n";
            cout << "======================================================\n";
            cout << "  1. League Operations & Fixtures\n";
            cout << "  2. Team & Personnel Management\n";
            cout << "  3. Stadium Operations & Ticketing\n";
            cout << "  4. Stadium Shop, Finance & Data\n";
            cout << "  5. Display Complete League Overview & Standings\n";
            cout << "  6. View Management Reports (Top Scorers, Attendance, Status)\n";
            cout << "  7. Log Out\n";
            cout << "======================================================\n";
            cout << "Enter choice (1-7): ";

            int ch;
            if (!(cin >> ch)) { clearInput(); continue; }
            clearInput();

            switch (ch) {
                case 1: showLeagueMenu(); break;
                case 2: showTeamPersonnelMenu(); break;
                case 3: showStadiumTicketingMenu(); break;
                case 4: showShopFinanceMenu(); break;
                case 5:
                    clearScreen();
                    league.displayFixtures();
                    league.displayStandings();
                    pausePrompt();
                    break;
                case 6:
                    showReportsMenu();
                    break;
                case 7:
                    currentUser = nullptr;
                    clearScreen();
                    cout << "\nLogged out successfully.\n";
                    pausePrompt();
                    return;
                default:
                    cout << "Invalid selection!\n";
                    pausePrompt();
            }
        }
    }

    // ==========================================
    // Three Management Reports Sub-Menu
    // ==========================================
    void showReportsMenu() {
        bool inReports = true;
        while (inReports) {
            clearScreen();
            cout << "======================================================\n";
            cout << "          LEAGUE MANAGEMENT REPORTS (SECTION 4.11)    \n";
            cout << "======================================================\n";
            cout << "  1. Report 1: Top Goal Scorers (Golden Boot Race)\n";
            cout << "  2. Report 2: Match Attendance & Stadium Capacity Utilization\n";
            cout << "  3. Report 3: Team Registration & Squad Summary\n";
            cout << "  4. Return to Previous Menu\n";
            cout << "======================================================\n";
            cout << "Enter choice (1-4): ";

            int ch;
            if (!(cin >> ch)) { clearInput(); continue; }
            clearInput();

            clearScreen();
            if (ch == 1) {
                league.generateTopScorersReport();
                pausePrompt();
            } else if (ch == 2) {
                league.generateAttendanceReport();
                pausePrompt();
            } else if (ch == 3) {
                league.generateTeamStatusReport();
                pausePrompt();
            } else if (ch == 4) {
                inReports = false;
            }
        }
    }

    // ==========================================
    // Person 1: League Operations Menu
    // ==========================================
    void showLeagueMenu() {
        bool inSubMenu = true;
        while (inSubMenu) {
            clearScreen();
            cout << "------------------------------------------------------\n";
            cout << "          LEAGUE OPERATIONS & FIXTURES MENU           \n";
            cout << "------------------------------------------------------\n";
            cout << "  1. View League Standings Table\n";
            cout << "  2. View All Fixtures\n";
            cout << "  3. Schedule New Fixture\n";
            cout << "  4. Assign Match Official to Fixture\n";
            cout << "  5. Record Match Result\n";
            cout << "  6. Commissioner: Inspect Fixture Readiness\n";
            cout << "  7. Commissioner: Suspend or Reinstate a Team\n";
            cout << "  8. View Management Reports\n";
            cout << "  9. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            cout << "------------------------------------------------------\n";
            cout << "Enter choice (1-9): ";

            int ch;
            if (!(cin >> ch)) { clearInput(); continue; }
            clearInput();

            try {
                if (ch == 1) {
                    clearScreen();
                    league.displayStandings();
                    pausePrompt();
                } else if (ch == 2) {
                    clearScreen();
                    league.displayFixtures();
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    cout << "=== SCHEDULE NEW FIXTURE ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* home = pickEligibleTeam("Home Team");
                    FootballTeam* away = pickEligibleTeam("Away Team", home);
                    Stadium* stad = pickStadium();

                    string date, time;
                    cout << "\n";
                    while (true) {
                        date = askDate("Enter Date (YYYY-MM-DD, e.g. 2026-12-05)");
                        time = askTime("Enter Time (HH:MM, e.g. 15:00)");
                        const string conflict = league.slotConflict(home, away, stad, date, time);
                        if (conflict.empty()) break;
                        cout << "[ERROR] " << conflict << " Enter another date and time.\n";
                    }

                    Fixture& fx = league.scheduleFixture(home, away, stad, date, time);
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    cout << "\n[SUCCESS] Scheduled " << fx.getFixtureId() << " (" << home->getName()
                              << " vs " << away->getName() << ") successfully!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    cout << "=== ASSIGN MATCH OFFICIAL ===\nType 'cancel' at any time to go back.\n\n";
                    Fixture& fx = pickOpenFixture();
                    const string fxId = fx.getFixtureId();
                    string offId;
                    while (true) {
                        const MatchOfficial* official = pickFrom("Official", league.getOfficials(), [](const MatchOfficial* o) {
                            return "[" + o->getId() + "] " + o->getName() + " (" + o->getRole() + ", " + o->getRank() + ")";
                        });
                        offId = official->getId();
                        if (official->isFree(fx.getDate(), fx.getTime())) break;
                        cout << "[ERROR] " << official->getName() << " is not free on " << fx.getDate()
                                  << " at " << fx.getTime() << ". Choose another.\n";
                    }

                    league.assignOfficial(fxId, offId);
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    cout << "\n[SUCCESS] Official assigned to fixture " << fxId << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    cout << "=== RECORD MATCH RESULT ===\nType 'cancel' at any time to go back.\n\n";
                    const string fxId = pickOpenFixture().getFixtureId();
                    int homeScore, awayScore;
                    cout << "\n";
                    homeScore = askInt("Enter Home Team Score", 0, 99);
                    awayScore = askInt("Enter Away Team Score", 0, 99);

                    league.recordResult(fxId, homeScore, awayScore);
                    // fixtures first: if the second save fails, the result still can't be counted twice
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    cout << "\n[SUCCESS] Match result recorded! Points table updated.\n";
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    cout << "=== INSPECT FIXTURE READINESS ===\nType 'cancel' at any time to go back.\n\n";
                    commissioner.inspectMatch(pickFixture());
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    cout << "=== SUSPEND / REINSTATE TEAM ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const bool suspend = pick("Action", {"Suspend", "Reinstate"}) == 0;

                    commissioner.suspendTeam(*t, suspend);
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    pausePrompt();
                } else if (ch == 8) {
                    showReportsMenu();
                } else if (ch == 9) {
                    if (currentUser->role != UserRole::ADMIN_ALL) {
                        currentUser = nullptr;
                        clearScreen();
                        cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const exception& e) {
                cout << "\n[LEAGUE ERROR] " << e.what() << "\n";
                pausePrompt();
            }
        }
    }

    // ==========================================
    // Person 2: Team & Personnel Menu
    // ==========================================
    void showTeamPersonnelMenu() {
        bool inSubMenu = true;
        while (inSubMenu) {
            clearScreen();
            cout << "------------------------------------------------------\n";
            cout << "           TEAM & PERSONNEL MANAGEMENT MENU           \n";
            cout << "------------------------------------------------------\n";
            cout << "  1. View All Registered Teams & Rosters\n";
            cout << "  2. Search Team Details\n";
            cout << "  3. Register New Football Team\n";
            cout << "  4. Add Player to Team Squad\n";
            cout << "  5. Add Coach to Team Staff\n";
            cout << "  6. Update Player Details\n";
            cout << "  7. Update Coach Details\n";
            cout << "  8. Remove Player from Team\n";
            cout << "  9. Remove Coach from Team\n";
            cout << "  10. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            cout << "------------------------------------------------------\n";
            cout << "Enter choice (1-10): ";

            int ch;
            if (!(cin >> ch)) { clearInput(); continue; }
            clearInput();

            try {
                if (ch == 1) {
                    clearScreen();
                    for (const auto* t : league.getTeams()) {
                        t->displayTeamInfo();
                    }
                    pausePrompt();
                } else if (ch == 2) {
                    clearScreen();
                    cout << "=== SEARCH TEAM DETAILS ===\nType 'cancel' at any time to go back.\n\n";
                    pickTeam()->displayTeamInfo();
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    cout << "=== REGISTER NEW TEAM ===\nType 'cancel' at any time to go back.\n\n";
                    string regNo, name, city;
                    cout << "Teams already registered:\n";
                    for (const auto* t : league.getTeams()) {
                        cout << "  - [" << t->getRegNumber() << "] " << t->getName() << " (" << t->getHomeCity() << ")\n";
                    }
                    cout << "\n";
                    regNo = askNewId("Enter Registration Number (e.g. T07)", league.getTeams(), [](const FootballTeam* t) {
                        return make_pair(t->getRegNumber(), t->getName());
                    });
                    name = askText("Enter Team Name");
                    city = pickParish("City/Parish");

                    auto newTeam = new FootballTeam(regNo, name, city);
                    league.addTeam(newTeam);
                    commissioner.approveTeam(*newTeam);
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    cout << "\n[SUCCESS] Registered " << name << "! (Saved to kpfl_teams.txt)\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    cout << "=== ADD PLAYER TO SQUAD ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    string pId, pName, pos, gender;
                    int age, jersey;

                    cout << "\nAdding player to " << t->getName() << ":\n";
                    pId = askNewId("Enter New Player ID (e.g. PL010)", t->getSquad(), [](const Player& x) {
                        return make_pair(x.getId(), x.getName());
                    });
                    pName = askText("Enter Player Name");
                    age = askInt("Enter Age (Rule: Must be >= 16)", 16, 55);
                    gender = pickGender();
                    pos = pickPosition();
                    jersey = askJersey(*t, "Enter Jersey Number (1-99, cannot conflict)", pId);

                    Player p(pId, pName, age, gender, pos, jersey);
                    t->addPlayer(p);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    cout << "\n[SUCCESS] Added player " << pName << " (#" << jersey << ") to " << t->getName() << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    cout << "=== ADD COACH TO TEAM ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    string cId, cName, qual, role, gender;
                    int age, exp;

                    cout << "\nAdding coach to " << t->getName() << ":\n";
                    cId = askNewId("Enter New Coach ID (e.g. CO03)", t->getCoachingStaff(), [](const Coach& x) {
                        return make_pair(x.getId(), x.getName());
                    });
                    cName = askText("Enter Coach Name");
                    age = askInt("Enter Age (Rule: Must be >= 21)", 21, 80);
                    gender = pickGender();
                    qual = pickQualification();
                    role = pickCoachRole();
                    exp = askInt("Enter Years Experience", 0, 80);

                    Coach c(cId, cName, age, gender, qual, role, exp);
                    t->addCoach(c);
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    cout << "\n[SUCCESS] Added coach " << cName << " to " << t->getName() << "!\n";
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    cout << "=== UPDATE PLAYER DETAILS ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const Player* p = &pickPlayer(*t);
                    const string pId = p->getId();

                    cout << "\nPress Enter to keep the current value.\n";
                    string name = askText("Name", p->getName());
                    int age = askInt("Age (>= 16)", 16, 55, to_string(p->getAge()));
                    string pos = pickPosition(p->getPosition());
                    int jersey = askJersey(*t, "Jersey Number (1-99)", pId, to_string(p->getJerseyNumber()));
                    string status = pickText("Status", {"Active", "Injured", "Benched", "Suspended"}, p->getPlayingStatus());

                    t->updatePlayer(pId, name, age, pos, jersey, status);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    cout << "\n[SUCCESS] Updated player " << name << " (#" << jersey << ").\n";
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    cout << "=== UPDATE COACH DETAILS ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const Coach& c = pickCoach(*t);
                    const string cId = c.getId();

                    cout << "\nPress Enter to keep the current value.\n";
                    const string name = askText("Name", c.getName());
                    const int age = askInt("Age (>= 21)", 21, 80, to_string(c.getAge()));
                    const string qual = pickQualification(c.getQualification());
                    const string role = pickCoachRole(c.getCoachingRole());
                    const int exp = askInt("Years Experience", 0, 80, to_string(c.getYearsExperience()));

                    t->updateCoach(cId, name, age, qual, role, exp);
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    cout << "\n[SUCCESS] Updated coach " << name << ".\n";
                    pausePrompt();
                } else if (ch == 8) {
                    clearScreen();
                    cout << "=== REMOVE PLAYER FROM SQUAD ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const string pId = pickPlayer(*t).getId();

                    t->removePlayer(pId);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    cout << "\n[SUCCESS] Removed player " << pId << " from squad.\n";
                    pausePrompt();
                } else if (ch == 9) {
                    clearScreen();
                    cout << "=== REMOVE COACH FROM TEAM ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const string cId = pickCoach(*t).getId();

                    t->removeCoach(cId);
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    cout << "\n[SUCCESS] Removed coach " << cId << " from " << t->getName() << ".\n";
                    pausePrompt();
                } else if (ch == 10) {
                    if (currentUser->role != UserRole::ADMIN_ALL) {
                        currentUser = nullptr;
                        clearScreen();
                        cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const exception& e) {
                cout << "\n[PERSONNEL ERROR] " << e.what() << "\n";
                pausePrompt();
            }
        }
    }

    // ==========================================
    // Person 3: Stadium Operations & Ticketing Menu
    // ==========================================
    void showStadiumTicketingMenu() {
        bool inSubMenu = true;
        while (inSubMenu) {
            clearScreen();
            cout << "------------------------------------------------------\n";
            cout << "         STADIUM OPERATIONS & TICKETING MENU          \n";
            cout << "------------------------------------------------------\n";
            cout << "  1. View All Stadiums\n";
            cout << "  2. Register New Stadium\n";
            cout << "  3. Update Stadium Capacity / Field Condition\n";
            cout << "  4. Generate Tickets for a Fixture\n";
            cout << "  5. Sell Ticket to Spectator\n";
            cout << "  6. View All Spectators\n";
            cout << "  7. Register New Spectator\n";
            cout << "  8. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            cout << "------------------------------------------------------\n";
            cout << "Enter choice (1-8): ";

            int ch;
            if (!(cin >> ch)) { clearInput(); continue; }
            clearInput();

            try {
                if (ch == 1) {
                    clearScreen();
                    for (const auto* s : league.getStadiums()) s->displayInfo();
                    pausePrompt();
                } else if (ch == 2) {
                    clearScreen();
                    cout << "=== REGISTER NEW STADIUM ===\nType 'cancel' at any time to go back.\n\n";
                    string sId, name, loc, cond;
                    int cap;
                    sId = askNewId("Enter Stadium ID (e.g. S03)", league.getStadiums(), [](const Stadium* x) {
                        return make_pair(x->getStadiumId(), x->getName());
                    });
                    name = askText("Enter Stadium Name");
                    loc = pickParish("Location");
                    cap = askInt("Enter Seating Capacity", 1, 1000000);
                    cond = pickCondition();

                    auto s = new Stadium(sId, name, loc, cap, cond);
                    league.addStadium(s);
                    DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
                    cout << "\n[SUCCESS] Registered stadium " << name << "!\n";
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    cout << "=== UPDATE STADIUM INFO ===\nType 'cancel' at any time to go back.\n\n";
                    Stadium* s = pickStadium();
                    int cap;
                    cout << "\n";
                    cap = askInt("Enter New Capacity (e.g. 40000)", 1, 1000000);
                    const string cond = pickCondition();

                    s->setSeatingCapacity(cap);
                    s->setFieldCondition(cond);
                    DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
                    cout << "\n[SUCCESS] Stadium " << s->getName() << " updated successfully!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    cout << "=== GENERATE TICKETS FOR FIXTURE ===\nType 'cancel' at any time to go back.\n\n";
                    Fixture* chosen = &pickFixture();
                    while (chosen->hasSoldTickets()) {
                        cout << "[ERROR] Some tickets for " << chosen->getFixtureId() << " are already sold. Choose another.\n";
                        chosen = &pickFixture();
                    }
                    Fixture& fx = *chosen;
                    const int capacity = fx.getStadium().getSeatingCapacity();
                    cout << "\nStadium capacity: " << capacity << " tickets in total.\n";
                    const int bleachers = askInt("Number of Bleachers Tickets ($1,500)", 0, capacity);
                    const int grandstand = askInt("Number of Grandstand Tickets ($3,500)", 0, capacity - bleachers);
                    const int vip = askInt("Number of VIP Tickets ($7,000)", 0, capacity - bleachers - grandstand);

                    fx.generateTickets(bleachers, grandstand, vip);
                    DataPersistence::saveTickets(league, "kpfl_tickets.txt");
                    cout << "\n[SUCCESS] Generated " << fx.getTickets().size() << " total tickets for " << fx.getFixtureId() << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    cout << "=== SELL TICKET TO SPECTATOR ===\nType 'cancel' at any time to go back.\n\n";

                    Fixture& fx = pickFixture();
                    if (fx.getTickets().empty()) {
                        cout << "\n[NOTICE] No tickets have been generated for fixture " << fx.getFixtureId() << " yet!\n";
                        cout << "Please choose Option 4 (Generate Tickets for a Fixture) first.\n";
                        pausePrompt();
                        continue;
                    }
                    Spectator* spectator = pickFrom("Spectator", league.getSpectators(), [](const Spectator* sp) {
                        return "[" + sp->getId() + "] " + sp->getName() + " (Tickets owned: " + to_string(sp->getTicketsBought()) + ")";
                    });
                    const vector<TicketCategory> categories = {TicketCategory::BLEACHERS, TicketCategory::GRANDSTAND, TicketCategory::VIP};
                    TicketCategory cat;
                    while (true) {
                        cat = pickFrom("Category", categories, [&](TicketCategory c) {
                            return ticketCategoryToString(c) + " (" + to_string(fx.ticketsLeft(c)) + " left)";
                        });
                        if (fx.ticketsLeft(cat) > 0) break;
                        cout << "[ERROR] " << ticketCategoryToString(cat) << " is sold out. Choose another.\n";
                    }

                    Ticket* t = fx.sellTicket(cat, *spectator);
                    // spectators first, so a saved ticket never refers to a missing buyer
                    DataPersistence::saveSpectators(league, "kpfl_spectators.txt");
                    DataPersistence::saveTickets(league, "kpfl_tickets.txt");
                    cout << "\n[SUCCESS] Ticket sold successfully!\n";
                    t->displayInfo();
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    for (const auto* sp : league.getSpectators()) sp->displayInfo();
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    cout << "=== REGISTER SPECTATOR ===\nType 'cancel' at any time to go back.\n\n";

                    string spId, name, gender, email, phone, prefTeam;
                    int age;
                    spId = askNewId("Enter New Spectator ID (e.g. SP03, SP04)", league.getSpectators(), [](const Spectator* x) {
                        return make_pair(x->getId(), x->getName());
                    });
                    name = askText("Enter Full Name");
                    age = askInt("Enter Age", 1, 120);
                    gender = pickGender();
                    email = askOptional("Enter Email Address");
                    phone = askOptional("Enter Phone Number");
                    vector<string> fanOptions;
                    for (const auto* t : league.getTeams()) fanOptions.push_back(t->getName());
                    fanOptions.push_back("Neutral");
                    prefTeam = pickText("Preferred Team", fanOptions);
                    if (prefTeam == "Neutral") prefTeam.clear();

                    auto sp = new Spectator(spId, name, age, gender, email, phone, prefTeam);
                    league.addSpectator(sp);
                    DataPersistence::saveSpectators(league, "kpfl_spectators.txt");
                    cout << "\n[SUCCESS] Spectator " << name << " registered successfully!\n";
                    pausePrompt();
                } else if (ch == 8) {
                    if (currentUser->role != UserRole::ADMIN_ALL) {
                        currentUser = nullptr;
                        clearScreen();
                        cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const exception& e) {
                cout << "\n[STADIUM/TICKETING ERROR] " << e.what() << "\n";
                pausePrompt();
            }
        }
    }

    // ==========================================
    // Person 4: Stadium Shop, Finance & Data Menu
    // ==========================================
    void showShopFinanceMenu() {
        bool inSubMenu = true;
        while (inSubMenu) {
            clearScreen();
            cout << "------------------------------------------------------\n";
            cout << "          STADIUM SHOP, FINANCE & DATA MENU           \n";
            cout << "------------------------------------------------------\n";
            cout << "  1. View Shop Inventory\n";
            cout << "  2. Add New Merchandise Item\n";
            cout << "  3. Add New Food / Beverage Item\n";
            cout << "  4. Process Customer Sale (Subtotal + 15% GCT)\n";
            cout << "  5. View Transaction History & Receipts\n";
            cout << "  6. Save All System Data to Files\n";
            cout << "  7. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            cout << "------------------------------------------------------\n";
            cout << "Enter choice (1-7): ";

            int ch;
            if (!(cin >> ch)) { clearInput(); continue; }
            clearInput();

            try {
                if (ch == 1) {
                    clearScreen();
                    shop.displayInventory();
                    pausePrompt();
                } else if (ch == 2) {
                    clearScreen();
                    cout << "=== ADD NEW MERCHANDISE ===\nType 'cancel' at any time to go back.\n\n";

                    string pId, name, size, team;
                    double price;
                    int stock;
                    pId = askNewId("Enter Product ID (e.g. M03, M04)", shop.getInventory(), [](const unique_ptr<Product>& x) {
                        return make_pair(x->getProductId(), x->getName());
                    });
                    name = askText("Enter Item Name (e.g. Cavalier SC Home Jersey)");
                    price = askMoney("Enter Price in JMD (e.g. 8500)");
                    stock = askInt("Enter Stock Quantity (e.g. 30)", 0, 1000000);
                    size = pickText("Size", {"S", "M", "L", "XL", "Standard"});
                    vector<string> teamOptions;
                    for (const auto* t : league.getTeams()) teamOptions.push_back("[" + t->getRegNumber() + "] " + t->getName());
                    teamOptions.push_back("League (general KPFL merchandise)");
                    const size_t teamIndex = pick("Linked Team", teamOptions);
                    team = teamIndex < league.getTeams().size() ? league.getTeams()[teamIndex]->getRegNumber() : "League";

                    shop.addProduct(make_unique<Merchandise>(pId, name, price, stock, size, team));
                    DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    cout << "\n[SUCCESS] Added merchandise '" << name << "' to shop inventory!\n";
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    cout << "=== ADD NEW FOOD / BEVERAGE ===\nType 'cancel' at any time to go back.\n\n";
                    string pId, name;
                    double price;
                    int stock;
                    pId = askNewId("Enter Product ID (e.g. F04)", shop.getInventory(), [](const unique_ptr<Product>& x) {
                        return make_pair(x->getProductId(), x->getName());
                    });
                    name = askText("Enter Item Name");
                    price = askMoney("Enter Price (JMD)");
                    stock = askInt("Enter Stock Quantity", 0, 1000000);
                    const bool isBev = pick("Type", {"Food", "Beverage"}) == 1;

                    shop.addProduct(make_unique<FoodItem>(pId, name, price, stock, isBev));
                    DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    cout << "\n[SUCCESS] Added food/beverage item to shop!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    cout << "=== PROCESS CUSTOMER SALE ===\nType 'cancel' at any time to go back.\n\n";
                    string custName;
                    custName = askText("Enter Customer Name");

                    vector<pair<string, int>> cart;
                    while (true) {
                        // stock not yet in this cart
                        const auto remaining = [&](const Product& p) {
                            int left = p.getStockQuantity();
                            for (const auto& item : cart) if (item.first == p.getProductId()) left -= item.second;
                            return left;
                        };
                        vector<string> options;
                        for (const auto& p : shop.getInventory()) {
                            ostringstream label;
                            label << "[" << p->getProductId() << "] " << p->getName() << " (" << money(p->getUnitPrice())
                                  << ", Stock: " << remaining(*p) << ")";
                            options.push_back(label.str());
                        }
                        options.push_back("Done - go to payment");
                        const size_t choice = pick("Product (" + to_string(cart.size()) + " in cart)", options);
                        if (choice == options.size() - 1) break;

                        const Product& product = *shop.getInventory()[choice];
                        const int left = remaining(product);
                        if (left <= 0) {
                            cout << "[ERROR] No more " << product.getName() << " in stock. Choose another.\n";
                            continue;
                        }
                        const int qty = askInt("Enter Quantity", 1, left);
                        cart.push_back({product.getProductId(), qty});
                    }

                    if (!cart.empty()) {
                        double cash;
                        const double due = shop.totalDue(cart);
                        // rounded up to the cent, so the amount shown is always enough
                        cout << "\nTotal due (including GCT): " << money(ceil(due * 100 - 1e-6) / 100) << "\n";
                        while ((cash = askMoney("Enter Cash Tendered (JMD)")) < due) {
                            cout << "[ERROR] That is less than the total due.\n";
                        }

                        clearScreen();
                        Receipt r = shop.processSale(custName, cart, cash);
                        StadiumShop::printReceipt(r);
                        DataPersistence::saveTransactions(shop, "kpfl_shop_receipts.txt");
                        DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    } else {
                        cout << "\nCart was empty. No transaction processed.\n";
                    }
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    const auto& history = shop.getTransactionHistory();
                    cout << "Transaction History (" << history.size() << " sales):\n";
                    for (const auto& r : history) {
                        StadiumShop::printReceipt(r);
                    }
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    DataPersistence::saveSpectators(league, "kpfl_spectators.txt");
                    DataPersistence::saveTickets(league, "kpfl_tickets.txt");
                    DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    DataPersistence::saveTransactions(shop, "kpfl_shop_receipts.txt");
                    cout << "\n[SUCCESS] System data exported to kpfl_*.txt files!\n";
                    pausePrompt();
                } else if (ch == 7) {
                    if (currentUser->role != UserRole::ADMIN_ALL) {
                        currentUser = nullptr;
                        clearScreen();
                        cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const exception& e) {
                cout << "\n[SHOP/FINANCE ERROR] " << e.what() << "\n";
                pausePrompt();
            }
        }
    }
};
