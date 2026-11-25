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
#include "AuthService.h"
#include "FootballLeague.h"
#include "LeagueCommissioner.h"
#include "FootballTeam.h"
#include "Stadium.h"
#include "StadiumShop.h"
#include "DataPersistence.h"

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
        std::system("cls");
#else
        std::system("clear");
#endif
    }

    void pausePrompt() const {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    void clearInput() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    static std::string toLower(std::string text) {
        for (char& ch : text) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        return text;
    }

    // reads one trimmed line; typing 'cancel' backs out of the form
    std::string readLine() {
        std::string in;
        if (!std::getline(std::cin, in)) throw InputCancelled{};
        const auto first = in.find_first_not_of(" \t\r");
        in = first == std::string::npos ? "" : in.substr(first, in.find_last_not_of(" \t\r") - first + 1);
        if (toLower(in) == "cancel") throw InputCancelled{};
        return in;
    }

    // keeps asking until check() returns no error message
    // if a current value is given, pressing Enter keeps it
    template <typename Check>
    std::string askUntil(const std::string& label, const std::string& current, Check check) {
        while (true) {
            std::cout << label << (current.empty() ? "" : " [" + current + "]") << ": ";
            std::string in = readLine();
            if (in.empty() && !current.empty()) return current;
            const std::string error = check(in);
            if (error.empty()) return in;
            std::cout << "[ERROR] " << error << "\n";
        }
    }

    std::string askText(const std::string& label, const std::string& current = "") {
        return askUntil(label, current, [](const std::string& in) {
            return std::string(in.empty() ? "This cannot be blank." : in.find('|') != std::string::npos ? "'|' is not allowed." : "");
        });
    }

    std::string askOptional(const std::string& label) {
        return askUntil(label, "", [](const std::string& in) {
            return std::string(in.find('|') != std::string::npos ? "'|' is not allowed." : "");
        });
    }

    std::string askId(const std::string& label) {
        return askUntil(label, "", [](const std::string& in) {
            return std::string(in.empty() || in.find_first_of(" \t|") != std::string::npos
                                   ? "Enter an ID without spaces or '|'." : "");
        });
    }

    int askInt(const std::string& label, int min, int max, const std::string& current = "") {
        const std::string text = askUntil(label, current, [&](const std::string& in) {
            std::istringstream ss(in);
            int v;
            const bool whole = (ss >> v) && (ss >> std::ws).eof();
            return std::string(whole && v >= min && v <= max ? ""
                : "Enter a whole number from " + std::to_string(min) + " to " + std::to_string(max) + ".");
        });
        return std::stoi(text);
    }

    double askMoney(const std::string& label) {
        const std::string text = askUntil(label, "", [](const std::string& in) {
            const bool ok = !in.empty() && in.size() <= 12 && in.find_first_not_of("0123456789.") == std::string::npos
                && in != "." && in.find('.') == in.rfind('.');
            return std::string(ok ? "" : "Enter an amount such as 8500 or 8500.50.");
        });
        return std::stod(text);
    }

    std::string askDate(const std::string& label) {
        return askUntil(label, "", [](const std::string& in) {
            return std::string(isValidDate(in) ? "" : "Enter a real date as YYYY-MM-DD, e.g. 2026-12-05.");
        });
    }

    std::string askTime(const std::string& label) {
        return askUntil(label, "", [](const std::string& in) {
            return std::string(isValidTime(in) ? "" : "Enter a 24-hour time as HH:MM, e.g. 15:00.");
        });
    }

    // shows a numbered list and returns the chosen index, asking again until the number is valid
    // if a current value is given, pressing Enter keeps it and -1 is returned
    int pick(const std::string& title, const std::vector<std::string>& options, const std::string& current = "") {
        if (options.empty()) throw std::runtime_error(title + ": nothing to choose from yet.");

        std::cout << "\n" << title << (current.empty() ? "" : " [" + current + "]") << ":\n";
        for (std::size_t i = 0; i < options.size(); ++i) {
            std::cout << "  " << i + 1 << ". " << options[i] << "\n";
        }
        while (true) {
            std::cout << "Enter number (1-" << options.size() << ", 0 to cancel"
                      << (current.empty() ? "" : ", Enter to keep") << "): ";
            const std::string in = readLine();
            if (in.empty() && !current.empty()) return -1;
            if (!in.empty() && in.size() <= 4 && in.find_first_not_of("0123456789") == std::string::npos) {
                const int n = std::stoi(in);
                if (n == 0) throw InputCancelled{};
                if (n <= static_cast<int>(options.size())) return n - 1;
            }
            std::cout << "[ERROR] Please choose a number from the list.\n";
        }
    }

    // same, but returns the chosen text
    std::string pickText(const std::string& title, const std::vector<std::string>& options, const std::string& current = "") {
        const int i = pick(title, options, current);
        return i < 0 ? current : options[i];
    }

    // same, with a last option for typing something not on the list
    std::string pickOrType(const std::string& title, std::vector<std::string> options, const std::string& current = "") {
        options.push_back("Other (type it in)");
        std::string choice = pickText(title, options, current);
        if (choice == options.back()) {
            choice = askText("Enter " + title);
        }
        return choice;
    }

    // picks one item from a collection, using label() to describe each
    template <typename Items, typename Label>
    auto& pickFrom(const std::string& title, Items& items, Label label) {
        std::vector<std::string> options;
        for (const auto& item : items) options.push_back(label(item));
        return items[pick(title, options)];
    }

    FootballTeam* pickTeam(const std::string& title = "Team", const FootballTeam* exclude = nullptr) {
        std::vector<FootballTeam*> teams;
        for (auto* t : league.getTeams()) if (t != exclude) teams.push_back(t);
        return pickFrom(title, teams, [](const FootballTeam* t) {
            return "[" + t->getRegNumber() + "] " + t->getName() + (t->isSuspended() ? " (SUSPENDED)" : "");
        });
    }

    Stadium* pickStadium() {
        return pickFrom("Stadium", league.getStadiums(), [](const Stadium* st) {
            return "[" + st->getStadiumId() + "] " + st->getName() + " (Capacity: "
                 + std::to_string(st->getSeatingCapacity()) + ", Condition: " + st->getFieldCondition() + ")";
        });
    }

    Fixture& pickFixture() {
        return *pickFrom("Fixture", league.getFixtures(), [](const std::unique_ptr<Fixture>& fx) {
            return "[" + fx->getFixtureId() + "] " + fx->getHome().getName() + " vs " + fx->getAway().getName()
                 + " (" + fx->getDate() + " " + fx->getTime() + " @ " + fx->getStadium().getName() + ", "
                 + (fx->getStatus() == FixtureStatus::COMPLETED ? "Completed" : "Scheduled")
                 + ", Officials: " + std::to_string(fx->getOfficials().size())
                 + ", Tickets: " + std::to_string(fx->getTickets().size()) + ")";
        });
    }

    Player& pickPlayer(FootballTeam& team) {
        return pickFrom("Player", team.getSquad(), [](const Player& p) {
            return "[" + p.getId() + "] #" + std::to_string(p.getJerseyNumber()) + " " + p.getName()
                 + " (" + p.getPosition() + ", " + p.getPlayingStatus() + ")";
        });
    }

    Coach& pickCoach(FootballTeam& team) {
        return pickFrom("Coach", team.getCoachingStaff(), [](const Coach& c) {
            return "[" + c.getId() + "] " + c.getName() + " (" + c.getCoachingRole() + ", "
                 + c.getQualification() + ", " + std::to_string(c.getYearsExperience()) + " yrs)";
        });
    }

    std::string pickGender() { return pick("Gender", {"Male", "Female"}) == 0 ? "M" : "F"; }

    std::string pickPosition(const std::string& current = "") {
        return pickText("Position", {"Forward", "Midfielder", "Defender", "Goalkeeper"}, current);
    }

    std::string pickQualification(const std::string& current = "") {
        return pickOrType("Qualification", {"CONCACAF Pro", "CONCACAF A", "CONCACAF B", "FIFA Elite License",
                                            "UEFA A License"}, current);
    }

    std::string pickCoachRole(const std::string& current = "") {
        return pickOrType("Role", {"Head Coach", "Assistant Coach", "Goalkeeping Coach", "Fitness Coach"}, current);
    }

    std::string pickCondition() {
        return pickText("Field Condition", {"Excellent", "Good", "Fair", "Maintenance"});
    }

    std::string pickParish(const std::string& title) {
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
                    std::cout << "\n======================================================\n";
                    std::cout << "  Exiting Jamaican Football League Management System. \n";
                    std::cout << "                    Goodbye!                          \n";
                    std::cout << "======================================================\n\n";
                    break;
                }
            } else {
                routeMenu();
            }
        }
    }

    bool showLoginPage() {
        clearScreen();
        std::cout << "======================================================\n";
        std::cout << "      KINGSTON PREMIER FOOTBALL LEAGUE (KPFL)        \n";
        std::cout << "                  SECURE LOGIN PORTAL                 \n";
        std::cout << "======================================================\n";
        auth.displayAvailableAccounts();

        std::cout << "Options:\n";
        std::cout << "  1. Log In\n";
        std::cout << "  2. Exit Application\n";
        std::cout << "Select option (1-2): ";

        int choice;
        if (!(std::cin >> choice)) {
            clearInput();
            return false;
        }
        clearInput();

        if (choice == 2) return false;

        std::string username, password;
        try {
            std::cout << "\nType 'cancel' to go back.\n";
            username = askText("Enter Username");
            password = askText("Enter Password");
        } catch (const InputCancelled&) {
            return true; // back to the login page
        }

        currentUser = auth.login(username, password);
        if (currentUser) {
            clearScreen();
            std::cout << "\n======================================================\n";
            std::cout << " [LOGIN SUCCESSFUL] Welcome, " << currentUser->displayName << "!\n";
            std::cout << "======================================================\n";
            pausePrompt();
            return true;
        } else {
            std::cout << "\n[ERROR] Invalid credentials! Please check your username and password.\n";
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
            std::cout << "======================================================\n";
            std::cout << "           SYSTEM ADMINISTRATOR MASTER DASHBOARD       \n";
            std::cout << "======================================================\n";
            std::cout << "  1. Person 1: League Operations & Fixtures Sub-Menu\n";
            std::cout << "  2. Person 2: Team & Personnel Management Sub-Menu\n";
            std::cout << "  3. Person 3: Stadium Operations & Ticketing Sub-Menu\n";
            std::cout << "  4. Person 4: Stadium Shop, Finance & Data Sub-Menu\n";
            std::cout << "  5. Display Complete League Overview & Standings\n";
            std::cout << "  6. View Management Reports (Top Scorers, Attendance, Status)\n";
            std::cout << "  7. Log Out\n";
            std::cout << "======================================================\n";
            std::cout << "Enter choice (1-7): ";

            int ch;
            if (!(std::cin >> ch)) { clearInput(); continue; }
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
                    std::cout << "\nLogged out successfully.\n";
                    pausePrompt();
                    return;
                default:
                    std::cout << "Invalid selection!\n";
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
            std::cout << "======================================================\n";
            std::cout << "          LEAGUE MANAGEMENT REPORTS (SECTION 4.11)    \n";
            std::cout << "======================================================\n";
            std::cout << "  1. Report 1: Top Goal Scorers (Golden Boot Race)\n";
            std::cout << "  2. Report 2: Match Attendance & Stadium Capacity Utilization\n";
            std::cout << "  3. Report 3: Team Registration & Squad Summary\n";
            std::cout << "  4. Return to Previous Menu\n";
            std::cout << "======================================================\n";
            std::cout << "Enter choice (1-4): ";

            int ch;
            if (!(std::cin >> ch)) { clearInput(); continue; }
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
            std::cout << "------------------------------------------------------\n";
            std::cout << "      [PERSON 1] LEAGUE OPERATIONS & FIXTURES MENU    \n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "  1. View League Standings Table\n";
            std::cout << "  2. View All Fixtures\n";
            std::cout << "  3. Schedule New Fixture\n";
            std::cout << "  4. Assign Match Official to Fixture\n";
            std::cout << "  5. Record Match Result\n";
            std::cout << "  6. Commissioner: Inspect Fixture Readiness\n";
            std::cout << "  7. Commissioner: Suspend or Reinstate a Team\n";
            std::cout << "  8. View Management Reports\n";
            std::cout << "  9. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "Enter choice (1-9): ";

            int ch;
            if (!(std::cin >> ch)) { clearInput(); continue; }
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
                    std::cout << "=== SCHEDULE NEW FIXTURE ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* home = pickTeam("Home Team");
                    FootballTeam* away = pickTeam("Away Team", home);
                    Stadium* stad = pickStadium();

                    std::string date, time;
                    std::cout << "\n";
                    date = askDate("Enter Date (YYYY-MM-DD, e.g. 2026-12-05)");
                    time = askTime("Enter Time (HH:MM, e.g. 15:00)");

                    Fixture& fx = league.scheduleFixture(home, away, stad, date, time);
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    std::cout << "\n[SUCCESS] Scheduled " << fx.getFixtureId() << " (" << home->getName()
                              << " vs " << away->getName() << ") successfully!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== ASSIGN MATCH OFFICIAL ===\nType 'cancel' at any time to go back.\n\n";
                    const std::string fxId = pickFixture().getFixtureId();
                    const std::string offId = pickFrom("Official", league.getOfficials(), [](const MatchOfficial* o) {
                        return "[" + o->getId() + "] " + o->getName() + " (" + o->getRole() + ", " + o->getRank() + ")";
                    })->getId();

                    league.assignOfficial(fxId, offId);
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    std::cout << "\n[SUCCESS] Official assigned to fixture " << fxId << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    std::cout << "=== RECORD MATCH RESULT ===\nType 'cancel' at any time to go back.\n\n";
                    const std::string fxId = pickFixture().getFixtureId();
                    int homeScore, awayScore;
                    std::cout << "\n";
                    homeScore = askInt("Enter Home Team Score", 0, 99);
                    awayScore = askInt("Enter Away Team Score", 0, 99);

                    league.recordResult(fxId, homeScore, awayScore);
                    // fixtures first: if the second save fails, the result still can't be counted twice
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    std::cout << "\n[SUCCESS] Match result recorded! Points table updated.\n";
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    std::cout << "=== INSPECT FIXTURE READINESS ===\nType 'cancel' at any time to go back.\n\n";
                    commissioner.inspectMatch(pickFixture());
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    std::cout << "=== SUSPEND / REINSTATE TEAM ===\nType 'cancel' at any time to go back.\n\n";
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
                        std::cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                std::cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const std::exception& e) {
                std::cout << "\n[LEAGUE ERROR] " << e.what() << "\n";
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
            std::cout << "------------------------------------------------------\n";
            std::cout << "      [PERSON 2] TEAM & PERSONNEL MANAGEMENT MENU     \n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "  1. View All Registered Teams & Rosters\n";
            std::cout << "  2. Search Team Details\n";
            std::cout << "  3. Register New Football Team\n";
            std::cout << "  4. Add Player to Team Squad\n";
            std::cout << "  5. Add Coach to Team Staff\n";
            std::cout << "  6. Update Player Details\n";
            std::cout << "  7. Update Coach Details\n";
            std::cout << "  8. Remove Player from Team\n";
            std::cout << "  9. Remove Coach from Team\n";
            std::cout << "  10. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "Enter choice (1-10): ";

            int ch;
            if (!(std::cin >> ch)) { clearInput(); continue; }
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
                    std::cout << "=== SEARCH TEAM DETAILS ===\nType 'cancel' at any time to go back.\n\n";
                    pickTeam()->displayTeamInfo();
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    std::cout << "=== REGISTER NEW TEAM ===\nType 'cancel' at any time to go back.\n\n";
                    std::string regNo, name, city;
                    std::cout << "Teams already registered:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - [" << t->getRegNumber() << "] " << t->getName() << " (" << t->getHomeCity() << ")\n";
                    }
                    std::cout << "\n";
                    while (true) {
                        regNo = askId("Enter Registration Number (e.g. T07)");
                        const FootballTeam* taken = nullptr;
                        for (const auto* t : league.getTeams()) {
                            if (toLower(t->getRegNumber()) == toLower(regNo)) taken = t; // t01 counts as T01
                        }
                        if (!taken) break;
                        std::cout << "[ERROR] " << regNo << " already belongs to " << taken->getName() << ". Choose another.\n";
                    }
                    name = askText("Enter Team Name");
                    city = pickParish("City/Parish");

                    auto newTeam = new FootballTeam(regNo, name, city);
                    league.addTeam(newTeam);
                    commissioner.approveTeam(*newTeam);
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    std::cout << "\n[SUCCESS] Registered " << name << "! (Saved to kpfl_teams.txt)\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== ADD PLAYER TO SQUAD ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    std::string pId, pName, pos, gender;
                    int age, jersey;

                    std::cout << "\nAdding player to " << t->getName() << ":\n";
                    pId = askId("Enter New Player ID (e.g. PL010)");
                    pName = askText("Enter Player Name");
                    age = askInt("Enter Age (Rule: Must be >= 16)", 16, 120);
                    gender = pickGender();
                    pos = pickPosition();
                    jersey = askInt("Enter Jersey Number (1-99, cannot conflict)", 1, 99);

                    Player p(pId, pName, age, gender, pos, jersey);
                    t->addPlayer(p);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    std::cout << "\n[SUCCESS] Added player " << pName << " (#" << jersey << ") to " << t->getName() << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    std::cout << "=== ADD COACH TO TEAM ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    std::string cId, cName, qual, role, gender;
                    int age, exp;

                    std::cout << "\nAdding coach to " << t->getName() << ":\n";
                    cId = askId("Enter New Coach ID (e.g. CO03)");
                    cName = askText("Enter Coach Name");
                    age = askInt("Enter Age (Rule: Must be >= 21)", 21, 120);
                    gender = pickGender();
                    qual = pickQualification();
                    role = pickCoachRole();
                    exp = askInt("Enter Years Experience", 0, 80);

                    Coach c(cId, cName, age, gender, qual, role, exp);
                    t->addCoach(c);
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    std::cout << "\n[SUCCESS] Added coach " << cName << " to " << t->getName() << "!\n";
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    std::cout << "=== UPDATE PLAYER DETAILS ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const Player* p = &pickPlayer(*t);
                    const std::string pId = p->getId();

                    std::cout << "\nPress Enter to keep the current value.\n";
                    std::string name = askText("Name", p->getName());
                    int age = askInt("Age (>= 16)", 16, 120, std::to_string(p->getAge()));
                    std::string pos = pickPosition(p->getPosition());
                    int jersey = askInt("Jersey Number (1-99)", 1, 99, std::to_string(p->getJerseyNumber()));
                    std::string status = pickText("Status", {"Active", "Injured", "Benched", "Suspended"}, p->getPlayingStatus());

                    t->updatePlayer(pId, name, age, pos, jersey, status);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    std::cout << "\n[SUCCESS] Updated player " << name << " (#" << jersey << ").\n";
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    std::cout << "=== UPDATE COACH DETAILS ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const Coach& c = pickCoach(*t);
                    const std::string cId = c.getId();

                    std::cout << "\nPress Enter to keep the current value.\n";
                    const std::string name = askText("Name", c.getName());
                    const int age = askInt("Age (>= 21)", 21, 120, std::to_string(c.getAge()));
                    const std::string qual = pickQualification(c.getQualification());
                    const std::string role = pickCoachRole(c.getCoachingRole());
                    const int exp = askInt("Years Experience", 0, 80, std::to_string(c.getYearsExperience()));

                    t->updateCoach(cId, name, age, qual, role, exp);
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    std::cout << "\n[SUCCESS] Updated coach " << name << ".\n";
                    pausePrompt();
                } else if (ch == 8) {
                    clearScreen();
                    std::cout << "=== REMOVE PLAYER FROM SQUAD ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const std::string pId = pickPlayer(*t).getId();

                    t->removePlayer(pId);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    std::cout << "\n[SUCCESS] Removed player " << pId << " from squad.\n";
                    pausePrompt();
                } else if (ch == 9) {
                    clearScreen();
                    std::cout << "=== REMOVE COACH FROM TEAM ===\nType 'cancel' at any time to go back.\n\n";
                    FootballTeam* t = pickTeam();
                    const std::string cId = pickCoach(*t).getId();

                    t->removeCoach(cId);
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    std::cout << "\n[SUCCESS] Removed coach " << cId << " from " << t->getName() << ".\n";
                    pausePrompt();
                } else if (ch == 10) {
                    if (currentUser->role != UserRole::ADMIN_ALL) {
                        currentUser = nullptr;
                        clearScreen();
                        std::cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                std::cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const std::exception& e) {
                std::cout << "\n[PERSONNEL ERROR] " << e.what() << "\n";
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
            std::cout << "------------------------------------------------------\n";
            std::cout << "    [PERSON 3] STADIUM OPERATIONS & TICKETING MENU    \n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "  1. View All Stadiums\n";
            std::cout << "  2. Register New Stadium\n";
            std::cout << "  3. Update Stadium Capacity / Field Condition\n";
            std::cout << "  4. Generate Tickets for a Fixture\n";
            std::cout << "  5. Sell Ticket to Spectator\n";
            std::cout << "  6. View All Spectators\n";
            std::cout << "  7. Register New Spectator\n";
            std::cout << "  8. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "Enter choice (1-8): ";

            int ch;
            if (!(std::cin >> ch)) { clearInput(); continue; }
            clearInput();

            try {
                if (ch == 1) {
                    clearScreen();
                    for (const auto* s : league.getStadiums()) s->displayInfo();
                    pausePrompt();
                } else if (ch == 2) {
                    clearScreen();
                    std::cout << "=== REGISTER NEW STADIUM ===\nType 'cancel' at any time to go back.\n\n";
                    std::string sId, name, loc, cond;
                    int cap;
                    sId = askId("Enter Stadium ID (e.g. S03)");
                    name = askText("Enter Stadium Name");
                    loc = pickParish("Location");
                    cap = askInt("Enter Seating Capacity", 1, 1000000);
                    cond = pickCondition();

                    auto s = new Stadium(sId, name, loc, cap, cond);
                    league.addStadium(s);
                    DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
                    std::cout << "\n[SUCCESS] Registered stadium " << name << "!\n";
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    std::cout << "=== UPDATE STADIUM INFO ===\nType 'cancel' at any time to go back.\n\n";
                    Stadium* s = pickStadium();
                    int cap;
                    std::cout << "\n";
                    cap = askInt("Enter New Capacity (e.g. 40000)", 1, 1000000);
                    const std::string cond = pickCondition();

                    s->setSeatingCapacity(cap);
                    s->setFieldCondition(cond);
                    DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
                    std::cout << "\n[SUCCESS] Stadium " << s->getName() << " updated successfully!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== GENERATE TICKETS FOR FIXTURE ===\nType 'cancel' at any time to go back.\n\n";
                    Fixture& fx = pickFixture();
                    int bleachers, grandstand, vip;
                    std::cout << "\n";
                    bleachers = askInt("Number of Bleachers Tickets ($1,500)", 0, 1000000);
                    grandstand = askInt("Number of Grandstand Tickets ($3,500)", 0, 1000000);
                    vip = askInt("Number of VIP Tickets ($7,000)", 0, 1000000);

                    fx.generateTickets(bleachers, grandstand, vip);
                    DataPersistence::saveTickets(league, "kpfl_tickets.txt");
                    std::cout << "\n[SUCCESS] Generated " << fx.getTickets().size() << " total tickets for " << fx.getFixtureId() << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    std::cout << "=== SELL TICKET TO SPECTATOR ===\nType 'cancel' at any time to go back.\n\n";

                    Fixture& fx = pickFixture();
                    if (fx.getTickets().empty()) {
                        std::cout << "\n[NOTICE] No tickets have been generated for fixture " << fx.getFixtureId() << " yet!\n";
                        std::cout << "Please choose Option 4 (Generate Tickets for a Fixture) first.\n";
                        pausePrompt();
                        continue;
                    }
                    Spectator* spectator = pickFrom("Spectator", league.getSpectators(), [](const Spectator* sp) {
                        return "[" + sp->getId() + "] " + sp->getName() + " (Tickets owned: " + std::to_string(sp->getTicketsBought()) + ")";
                    });
                    const std::vector<TicketCategory> categories = {TicketCategory::BLEACHERS, TicketCategory::GRANDSTAND, TicketCategory::VIP};
                    const TicketCategory cat = pickFrom("Category", categories, [](TicketCategory c) { return ticketCategoryToString(c); });

                    Ticket* t = fx.sellTicket(cat, *spectator);
                    // spectators first, so a saved ticket never refers to a missing buyer
                    DataPersistence::saveSpectators(league, "kpfl_spectators.txt");
                    DataPersistence::saveTickets(league, "kpfl_tickets.txt");
                    std::cout << "\n[SUCCESS] Ticket sold successfully!\n";
                    t->displayInfo();
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    for (const auto* sp : league.getSpectators()) sp->displayInfo();
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    std::cout << "=== REGISTER SPECTATOR ===\nType 'cancel' at any time to go back.\n\n";

                    std::string spId, name, gender, email, phone, prefTeam;
                    int age;
                    spId = askId("Enter New Spectator ID (e.g. SP03, SP04)");
                    name = askText("Enter Full Name");
                    age = askInt("Enter Age", 1, 120);
                    gender = pickGender();
                    email = askOptional("Enter Email Address");
                    phone = askOptional("Enter Phone Number");
                    std::vector<std::string> fanOptions;
                    for (const auto* t : league.getTeams()) fanOptions.push_back(t->getName());
                    fanOptions.push_back("Neutral");
                    prefTeam = pickText("Preferred Team", fanOptions);
                    if (prefTeam == "Neutral") prefTeam.clear();

                    auto sp = new Spectator(spId, name, age, gender, email, phone, prefTeam);
                    league.addSpectator(sp);
                    DataPersistence::saveSpectators(league, "kpfl_spectators.txt");
                    std::cout << "\n[SUCCESS] Spectator " << name << " registered successfully!\n";
                    pausePrompt();
                } else if (ch == 8) {
                    if (currentUser->role != UserRole::ADMIN_ALL) {
                        currentUser = nullptr;
                        clearScreen();
                        std::cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                std::cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const std::exception& e) {
                std::cout << "\n[STADIUM/TICKETING ERROR] " << e.what() << "\n";
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
            std::cout << "------------------------------------------------------\n";
            std::cout << "    [PERSON 4] STADIUM SHOP, FINANCE & DATA MENU      \n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "  1. View Shop Inventory\n";
            std::cout << "  2. Add New Merchandise Item\n";
            std::cout << "  3. Add New Food / Beverage Item\n";
            std::cout << "  4. Process Customer Sale (Subtotal + 15% GCT)\n";
            std::cout << "  5. View Transaction History & Receipts\n";
            std::cout << "  6. Save All System Data to Files\n";
            std::cout << "  7. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "Enter choice (1-7): ";

            int ch;
            if (!(std::cin >> ch)) { clearInput(); continue; }
            clearInput();

            try {
                if (ch == 1) {
                    clearScreen();
                    shop.displayInventory();
                    pausePrompt();
                } else if (ch == 2) {
                    clearScreen();
                    std::cout << "=== ADD NEW MERCHANDISE ===\nType 'cancel' at any time to go back.\n\n";

                    std::string pId, name, size, team;
                    double price;
                    int stock;
                    pId = askId("Enter Product ID (e.g. M03, M04)");
                    name = askText("Enter Item Name (e.g. Cavalier SC Home Jersey)");
                    price = askMoney("Enter Price in JMD (e.g. 8500)");
                    stock = askInt("Enter Stock Quantity (e.g. 30)", 0, 1000000);
                    size = pickText("Size", {"S", "M", "L", "XL", "Standard"});
                    std::vector<std::string> teamOptions;
                    for (const auto* t : league.getTeams()) teamOptions.push_back("[" + t->getRegNumber() + "] " + t->getName());
                    teamOptions.push_back("League (general KPFL merchandise)");
                    const std::size_t teamIndex = pick("Linked Team", teamOptions);
                    team = teamIndex < league.getTeams().size() ? league.getTeams()[teamIndex]->getRegNumber() : "League";

                    shop.addProduct(std::make_unique<Merchandise>(pId, name, price, stock, size, team));
                    DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    std::cout << "\n[SUCCESS] Added merchandise '" << name << "' to shop inventory!\n";
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    std::cout << "=== ADD NEW FOOD / BEVERAGE ===\nType 'cancel' at any time to go back.\n\n";
                    std::string pId, name;
                    double price;
                    int stock;
                    pId = askId("Enter Product ID (e.g. F04)");
                    name = askText("Enter Item Name");
                    price = askMoney("Enter Price (JMD)");
                    stock = askInt("Enter Stock Quantity", 0, 1000000);
                    const bool isBev = pick("Type", {"Food", "Beverage"}) == 1;

                    shop.addProduct(std::make_unique<FoodItem>(pId, name, price, stock, isBev));
                    DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    std::cout << "\n[SUCCESS] Added food/beverage item to shop!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== PROCESS CUSTOMER SALE ===\nType 'cancel' at any time to go back.\n\n";
                    std::string custName;
                    custName = askText("Enter Customer Name");

                    std::vector<std::pair<std::string, int>> cart;
                    while (true) {
                        std::vector<std::string> options;
                        for (const auto& p : shop.getInventory()) {
                            std::ostringstream label;
                            label << "[" << p->getProductId() << "] " << p->getName() << " (JMD $" << std::fixed << std::setprecision(2)
                                  << p->getUnitPrice() << ", Stock: " << p->getStockQuantity() << ")";
                            options.push_back(label.str());
                        }
                        options.push_back("Done - go to payment");
                        const std::size_t choice = pick("Product (" + std::to_string(cart.size()) + " in cart)", options);
                        if (choice == options.size() - 1) break;

                        const int qty = askInt("Enter Quantity", 1, 1000000);
                        cart.push_back({shop.getInventory()[choice]->getProductId(), qty});
                    }

                    if (!cart.empty()) {
                        double cash;
                        std::cout << "\n";
                        cash = askMoney("Enter Cash Tendered (JMD)");

                        clearScreen();
                        Receipt r = shop.processSale(custName, cart, cash);
                        StadiumShop::printReceipt(r);
                        DataPersistence::saveTransactions(shop, "kpfl_shop_receipts.txt");
                        DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    } else {
                        std::cout << "\nCart was empty. No transaction processed.\n";
                    }
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    const auto& history = shop.getTransactionHistory();
                    std::cout << "Transaction History (" << history.size() << " sales):\n";
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
                    std::cout << "\n[SUCCESS] System data exported to kpfl_*.txt files!\n";
                    pausePrompt();
                } else if (ch == 7) {
                    if (currentUser->role != UserRole::ADMIN_ALL) {
                        currentUser = nullptr;
                        clearScreen();
                        std::cout << "\nLogged out successfully.\n";
                        pausePrompt();
                    }
                    inSubMenu = false;
                }
            } catch (const InputCancelled&) {
                std::cout << "\nCancelled. Nothing was changed.\n";
                pausePrompt();
            } catch (const std::exception& e) {
                std::cout << "\n[SHOP/FINANCE ERROR] " << e.what() << "\n";
                pausePrompt();
            }
        }
    }
};
