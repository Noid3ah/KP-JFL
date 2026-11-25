#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <limits>
#include <cstdlib>
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

        if (choice == 2) return false;

        std::string username, password;
        std::cout << "\nEnter Username: ";
        std::cin >> username;
        std::cout << "Enter Password: ";
        std::cin >> password;
        clearInput();

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
                    std::cout << "=== SCHEDULE NEW FIXTURE ===\n\n";
                    std::cout << "Available Teams:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - ID: [" << t->getRegNumber() << "] " << t->getName()
                                  << (t->isSuspended() ? " [SUSPENDED - Cannot be scheduled]" : " [ACTIVE]") << "\n";
                    }

                    std::cout << "\nAvailable Stadiums:\n";
                    for (const auto* s : league.getStadiums()) {
                        std::cout << "  - ID: [" << s->getStadiumId() << "] " << s->getName()
                                  << " (Cap: " << s->getSeatingCapacity() << " | Cond: " << s->getFieldCondition() << ")\n";
                    }

                    std::string homeReg, awayReg, stadId, date, time;
                    std::cout << "\nEnter Home Team ID (e.g. T01): "; std::cin >> homeReg;
                    std::cout << "Enter Away Team ID (e.g. T02): "; std::cin >> awayReg;
                    std::cout << "Enter Stadium ID (e.g. S01): "; std::cin >> stadId;
                    std::cout << "Enter Date (YYYY-MM-DD, e.g. 2026-12-05): "; std::cin >> date;
                    std::cout << "Enter Time (HH:MM, e.g. 15:00): "; std::cin >> time;
                    clearInput();

                    FootballTeam* home = league.findTeam(homeReg);
                    FootballTeam* away = league.findTeam(awayReg);
                    Stadium* stad = league.findStadium(stadId);

                    if (!home || !away || !stad) {
                        std::cout << "\n[ERROR] Invalid Team or Stadium ID entered!\n";
                    } else {
                        Fixture& fx = league.scheduleFixture(home, away, stad, date, time);
                        DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                        std::cout << "\n[SUCCESS] Scheduled " << fx.getFixtureId() << " (" << home->getName()
                                  << " vs " << away->getName() << ") successfully!\n";
                    }
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== ASSIGN MATCH OFFICIAL ===\n\n";
                    std::cout << "Current Fixtures:\n";
                    for (const auto& fx : league.getFixtures()) {
                        std::cout << "  - ID: [" << fx->getFixtureId() << "] " << fx->getHome().getName()
                                  << " vs " << fx->getAway().getName()
                                  << " (" << fx->getDate() << " " << fx->getTime() << " @ " << fx->getStadium().getName() << ")\n";
                    }
                    std::cout << "\nAvailable Officials:\n";
                    for (const auto* o : league.getOfficials()) {
                        std::cout << "  - ID: [" << o->getId() << "] " << o->getName()
                                  << " (" << o->getRole() << " | Rank: " << o->getRank() << ")\n";
                    }
                    std::string fxId, offId;
                    std::cout << "\nEnter Fixture ID (e.g. FX001): "; std::cin >> fxId;
                    std::cout << "Enter Official ID (e.g. O001): "; std::cin >> offId;
                    clearInput();

                    league.assignOfficial(fxId, offId);
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    std::cout << "\n[SUCCESS] Official assigned to fixture " << fxId << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    std::cout << "=== RECORD MATCH RESULT ===\n\n";
                    std::cout << "Fixtures Awaiting Results / In League:\n";
                    for (const auto& fx : league.getFixtures()) {
                        std::cout << "  - ID: [" << fx->getFixtureId() << "] " << fx->getHome().getName()
                                  << " vs " << fx->getAway().getName()
                                  << " [" << (fx->getStatus() == FixtureStatus::COMPLETED ? "COMPLETED" : "SCHEDULED") << "]\n";
                    }

                    std::string fxId;
                    int homeScore, awayScore;
                    std::cout << "\nEnter Fixture ID (e.g. FX001): "; std::cin >> fxId;
                    std::cout << "Enter Home Team Score: "; std::cin >> homeScore;
                    std::cout << "Enter Away Team Score: "; std::cin >> awayScore;
                    clearInput();

                    league.recordResult(fxId, homeScore, awayScore);
                    // fixtures first: if the second save fails, the result still can't be counted twice
                    DataPersistence::saveFixtures(league, "kpfl_fixtures.txt");
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    std::cout << "\n[SUCCESS] Match result recorded! Points table updated.\n";
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    std::cout << "=== INSPECT FIXTURE READINESS ===\n\n";
                    std::cout << "Current Fixtures:\n";
                    for (const auto& fx : league.getFixtures()) {
                        std::cout << "  - ID: [" << fx->getFixtureId() << "] " << fx->getHome().getName()
                                  << " vs " << fx->getAway().getName()
                                  << " (Officials assigned: " << fx->getOfficials().size() << ")\n";
                    }
                    std::string fxId;
                    std::cout << "\nEnter Fixture ID to inspect: "; std::cin >> fxId;
                    clearInput();

                    bool found = false;
                    for (const auto& fx : league.getFixtures()) {
                        if (fx->getFixtureId() == fxId) {
                            commissioner.inspectMatch(*fx);
                            found = true;
                            break;
                        }
                    }
                    if (!found) std::cout << "\n[ERROR] Fixture not found!\n";
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    std::cout << "=== SUSPEND / REINSTATE TEAM ===\n\n";
                    std::cout << "Registered Teams:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - ID: [" << t->getRegNumber() << "] " << t->getName()
                                  << " | Current Status: " << (t->isSuspended() ? "SUSPENDED" : "ACTIVE") << "\n";
                    }
                    std::string teamId;
                    int statusChoice;
                    std::cout << "\nEnter Team Reg Number (e.g. T01, T04): "; std::cin >> teamId;
                    std::cout << "Select Action (1: Suspend | 2: Reinstate): "; std::cin >> statusChoice;
                    clearInput();

                    FootballTeam* t = league.findTeam(teamId);
                    if (t) {
                        commissioner.suspendTeam(*t, statusChoice == 1);
                        DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    } else {
                        std::cout << "\n[ERROR] Team not found!\n";
                    }
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
            std::cout << "  2. Register New Football Team\n";
            std::cout << "  3. Add Player to Team Squad\n";
            std::cout << "  4. Add Coach to Team Staff\n";
            std::cout << "  5. Remove Player from Team\n";
            std::cout << "  6. Search Team Details\n";
            std::cout << "  7. Update Player Details\n";
            std::cout << "  8. " << (currentUser->role == UserRole::ADMIN_ALL ? "Return to Master Menu" : "Log Out") << "\n";
            std::cout << "------------------------------------------------------\n";
            std::cout << "Enter choice (1-8): ";

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
                    std::cout << "=== REGISTER NEW TEAM ===\n\n";
                    std::string regNo, name, city;
                    std::cout << "Enter Registration Number (e.g. T07): "; std::cin >> regNo;
                    clearInput();
                    std::cout << "Enter Team Name: "; std::getline(std::cin, name);
                    std::cout << "Enter City/Parish: "; std::getline(std::cin, city);

                    auto newTeam = new FootballTeam(regNo, name, city);
                    league.addTeam(newTeam);
                    commissioner.approveTeam(*newTeam);
                    DataPersistence::saveTeams(league, "kpfl_teams.txt");
                    std::cout << "\n[SUCCESS] Registered " << name << "! (Saved to kpfl_teams.txt)\n";
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    std::cout << "=== ADD PLAYER TO SQUAD ===\n\n";
                    std::cout << "Available Teams:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - ID: [" << t->getRegNumber() << "] " << t->getName()
                                  << " (Current Squad: " << t->getSquad().size() << " players)\n";
                    }

                    std::string teamId, pId, pName, pos, gender;
                    int age, jersey;
                    std::cout << "\nEnter Team Reg Number (e.g. T01): "; std::cin >> teamId;
                    FootballTeam* t = league.findTeam(teamId);
                    if (!t) {
                        std::cout << "\n[ERROR] Team '" << teamId << "' not found!\n";
                        pausePrompt();
                        continue;
                    }

                    std::cout << "\nAdding player to " << t->getName() << ":\n";
                    std::cout << "Enter New Player ID (e.g. PL010): "; std::cin >> pId;
                    clearInput();
                    std::cout << "Enter Player Name: "; std::getline(std::cin, pName);
                    std::cout << "Enter Age (Rule: Must be >= 16): "; std::cin >> age;
                    std::cout << "Enter Gender (M/F): "; std::cin >> gender;
                    clearInput();
                    std::cout << "Enter Position (Forward / Midfielder / Defender / Goalkeeper): "; std::getline(std::cin, pos);
                    std::cout << "Enter Jersey Number (1-99, cannot conflict): "; std::cin >> jersey;
                    clearInput();

                    Player p(pId, pName, age, gender, pos, jersey);
                    t->addPlayer(p);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    std::cout << "\n[SUCCESS] Added player " << pName << " (#" << jersey << ") to " << t->getName() << "!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== ADD COACH TO TEAM ===\n\n";
                    std::cout << "Available Teams:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - ID: [" << t->getRegNumber() << "] " << t->getName()
                                  << " (Coaches: " << t->getCoachingStaff().size() << ")\n";
                    }

                    std::string teamId, cId, cName, qual, role, gender;
                    int age, exp;
                    std::cout << "\nEnter Team Reg Number (e.g. T01): "; std::cin >> teamId;
                    FootballTeam* t = league.findTeam(teamId);
                    if (!t) {
                        std::cout << "\n[ERROR] Team '" << teamId << "' not found!\n";
                        pausePrompt();
                        continue;
                    }

                    std::cout << "\nAdding coach to " << t->getName() << ":\n";
                    std::cout << "Enter New Coach ID (e.g. CO03): "; std::cin >> cId;
                    clearInput();
                    std::cout << "Enter Coach Name: "; std::getline(std::cin, cName);
                    std::cout << "Enter Age (Rule: Must be >= 21): "; std::cin >> age;
                    std::cout << "Enter Gender (M/F): "; std::cin >> gender;
                    clearInput();
                    std::cout << "Enter Qualification (e.g. CONCACAF Pro / FIFA Elite): "; std::getline(std::cin, qual);
                    std::cout << "Enter Role (e.g. Head Coach / Assistant Coach): "; std::getline(std::cin, role);
                    std::cout << "Enter Years Experience: "; std::cin >> exp;
                    clearInput();

                    Coach c(cId, cName, age, gender, qual, role, exp);
                    t->addCoach(c);
                    DataPersistence::saveCoaches(league, "kpfl_coaches.txt");
                    std::cout << "\n[SUCCESS] Added coach " << cName << " to " << t->getName() << "!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    std::cout << "=== REMOVE PLAYER FROM SQUAD ===\n\n";
                    std::cout << "Select from Teams:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - ID: [" << t->getRegNumber() << "] " << t->getName() << "\n";
                    }

                    std::string teamId, pId;
                    std::cout << "\nEnter Team Reg Number: "; std::cin >> teamId;
                    FootballTeam* t = league.findTeam(teamId);
                    if (!t) {
                        std::cout << "\n[ERROR] Team not found!\n";
                        pausePrompt();
                        continue;
                    }

                    std::cout << "\nCurrent Players in " << t->getName() << ":\n";
                    for (const auto& p : t->getSquad()) {
                        std::cout << "  - ID: [" << p.getId() << "] #" << p.getJerseyNumber() << " " << p.getName() << "\n";
                    }

                    std::cout << "\nEnter Player ID to remove: "; std::cin >> pId;
                    clearInput();

                    if (t->removePlayer(pId)) {
                        DataPersistence::savePlayers(league, "kpfl_players.txt");
                        std::cout << "\n[SUCCESS] Removed player " << pId << " from squad.\n";
                    } else {
                        std::cout << "\n[ERROR] Player ID '" << pId << "' not found in squad.\n";
                    }
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    std::cout << "=== SEARCH TEAM DETAILS ===\n\n";
                    std::cout << "Registered Teams:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - ID: [" << t->getRegNumber() << "] " << t->getName() << "\n";
                    }
                    std::string teamId;
                    std::cout << "\nEnter Team Reg Number to inspect: "; std::cin >> teamId;
                    clearInput();

                    FootballTeam* t = league.findTeam(teamId);
                    if (t) t->displayTeamInfo();
                    else std::cout << "\n[ERROR] Team not found!\n";
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    std::cout << "=== UPDATE PLAYER DETAILS ===\n\n";
                    std::cout << "Select from Teams:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - ID: [" << t->getRegNumber() << "] " << t->getName() << "\n";
                    }

                    std::string teamId, pId;
                    std::cout << "\nEnter Team Reg Number: "; std::cin >> teamId;
                    clearInput();
                    FootballTeam* t = league.findTeam(teamId);
                    if (!t) {
                        std::cout << "\n[ERROR] Team not found!\n";
                        pausePrompt();
                        continue;
                    }

                    std::cout << "\nCurrent Players in " << t->getName() << ":\n";
                    for (const auto& p : t->getSquad()) p.displayInfo();

                    std::cout << "\nEnter Player ID to update: "; std::cin >> pId;
                    clearInput();
                    const Player* p = t->findPlayer(pId);
                    if (!p) {
                        std::cout << "\n[ERROR] Player ID '" << pId << "' not found in squad.\n";
                        pausePrompt();
                        continue;
                    }

                    // blank input keeps the current value
                    auto ask = [](const std::string& label, const std::string& current) {
                        std::string in;
                        std::cout << label << " [" << current << "]: ";
                        std::getline(std::cin, in);
                        return in.empty() ? current : in;
                    };
                    auto askInt = [&](const std::string& label, int current) {
                        std::istringstream ss(ask(label, std::to_string(current)));
                        int v;
                        if (!(ss >> v) || !(ss >> std::ws).eof()) {
                            throw std::invalid_argument(label + " must be a whole number.");
                        }
                        return v;
                    };

                    std::cout << "\nPress Enter to keep the current value.\n";
                    std::string name = ask("Name", p->getName());
                    int age = askInt("Age (>= 16)", p->getAge());
                    std::string pos = ask("Position (Forward / Midfielder / Defender / Goalkeeper)", p->getPosition());
                    int jersey = askInt("Jersey Number (1-99)", p->getJerseyNumber());
                    std::string status = ask("Status (Active / Injured / Benched / Suspended)", p->getPlayingStatus());

                    t->updatePlayer(pId, name, age, pos, jersey, status);
                    DataPersistence::savePlayers(league, "kpfl_players.txt");
                    std::cout << "\n[SUCCESS] Updated player " << name << " (#" << jersey << ").\n";
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
            std::cout << "  6. Register New Spectator\n";
            std::cout << "  7. View All Spectators\n";
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
                    std::cout << "=== REGISTER NEW STADIUM ===\n\n";
                    std::string sId, name, loc, cond;
                    int cap;
                    std::cout << "Enter Stadium ID (e.g. S03): "; std::cin >> sId;
                    clearInput();
                    std::cout << "Enter Stadium Name: "; std::getline(std::cin, name);
                    std::cout << "Enter Location: "; std::getline(std::cin, loc);
                    std::cout << "Enter Seating Capacity: "; std::cin >> cap;
                    clearInput();
                    std::cout << "Enter Condition (Excellent/Good/Fair): "; std::getline(std::cin, cond);

                    auto s = new Stadium(sId, name, loc, cap, cond);
                    league.addStadium(s);
                    DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
                    std::cout << "\n[SUCCESS] Registered stadium " << name << "!\n";
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    std::cout << "=== UPDATE STADIUM INFO ===\n\n";
                    std::cout << "Registered Stadiums:\n";
                    for (const auto* s : league.getStadiums()) {
                        std::cout << "  - ID: [" << s->getStadiumId() << "] " << s->getName()
                                  << " (Current Cap: " << s->getSeatingCapacity() << " | Cond: " << s->getFieldCondition() << ")\n";
                    }

                    std::string sId, cond;
                    int cap;
                    std::cout << "\nEnter Stadium ID to update: "; std::cin >> sId;
                    Stadium* s = league.findStadium(sId);
                    if (!s) {
                        std::cout << "\n[ERROR] Stadium '" << sId << "' not found!\n";
                        pausePrompt();
                        continue;
                    }
                    std::cout << "Enter New Capacity (e.g. 40000): "; std::cin >> cap;
                    clearInput();
                    std::cout << "Enter New Condition (Excellent / Good / Fair / Maintenance): "; std::getline(std::cin, cond);

                    s->setSeatingCapacity(cap);
                    s->setFieldCondition(cond);
                    DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
                    std::cout << "\n[SUCCESS] Stadium " << s->getName() << " updated successfully!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== GENERATE TICKETS FOR FIXTURE ===\n\n";
                    std::cout << "Current Fixtures:\n";
                    for (const auto& fx : league.getFixtures()) {
                        std::cout << "  - ID: [" << fx->getFixtureId() << "] " << fx->getHome().getName()
                                  << " vs " << fx->getAway().getName()
                                  << " @ " << fx->getStadium().getName()
                                  << " (Stadium Cap: " << fx->getStadium().getSeatingCapacity()
                                  << " | Existing Tickets: " << fx->getTickets().size() << ")\n";
                    }

                    std::string fxId;
                    int bleachers, grandstand, vip;
                    std::cout << "\nEnter Fixture ID (e.g. FX001): "; std::cin >> fxId;
                    std::cout << "Number of Bleachers Tickets ($1,500): "; std::cin >> bleachers;
                    std::cout << "Number of Grandstand Tickets ($3,500): "; std::cin >> grandstand;
                    std::cout << "Number of VIP Tickets ($7,000): "; std::cin >> vip;
                    clearInput();

                    bool found = false;
                    for (const auto& fx : league.getFixtures()) {
                        if (fx->getFixtureId() == fxId) {
                            fx->generateTickets(bleachers, grandstand, vip);
                            DataPersistence::saveTickets(league, "kpfl_tickets.txt");
                            std::cout << "\n[SUCCESS] Generated " << fx->getTickets().size() << " total tickets for " << fx->getFixtureId() << "!\n";
                            found = true;
                            break;
                        }
                    }
                    if (!found) std::cout << "\n[ERROR] Fixture '" << fxId << "' not found!\n";
                    pausePrompt();
                } else if (ch == 5) {
                    clearScreen();
                    std::cout << "=== SELL TICKET TO SPECTATOR ===\n\n";

                    std::cout << "Available Fixtures:\n";
                    for (const auto& fx : league.getFixtures()) {
                        std::cout << "  - " << fx->getFixtureId() << ": " << fx->getHome().getName()
                                  << " vs " << fx->getAway().getName()
                                  << " (Tickets: " << fx->getTickets().size() << " created)\n";
                    }

                    std::cout << "\nAvailable Registered Spectators:\n";
                    if (league.getSpectators().empty()) {
                        std::cout << "  (No spectators registered yet. Please use Option 6 first!)\n";
                    } else {
                        for (const auto* sp : league.getSpectators()) {
                            std::cout << "  - ID: [" << sp->getId() << "] " << sp->getName()
                                      << " (Tickets already owned: " << sp->getTicketsBought() << ")\n";
                        }
                    }

                    std::string fxId, spId;
                    int catChoice;
                    std::cout << "\nEnter Fixture ID (e.g. FX001): "; std::cin >> fxId;
                    std::cout << "Enter Spectator ID (e.g. SP01, SP02, SP03): "; std::cin >> spId;
                    std::cout << "Select Category (1. Bleachers | 2. Grandstand | 3. VIP): "; std::cin >> catChoice;
                    clearInput();

                    TicketCategory cat = TicketCategory::BLEACHERS;
                    if (catChoice == 2) cat = TicketCategory::GRANDSTAND;
                    else if (catChoice == 3) cat = TicketCategory::VIP;

                    // Trim and match spectator ID (case-insensitive for convenience)
                    Spectator* spectator = nullptr;
                    for (auto* sp : league.getSpectators()) {
                        std::string a = sp->getId();
                        std::string b = spId;
                        std::transform(a.begin(), a.end(), a.begin(), ::toupper);
                        std::transform(b.begin(), b.end(), b.begin(), ::toupper);
                        if (a == b) { spectator = sp; break; }
                    }

                    if (!spectator) {
                        std::cout << "\n[ERROR] Spectator with ID '" << spId << "' was not found!\n";
                        std::cout << "Hint: Check the list of available spectator IDs printed above.\n";
                        pausePrompt();
                        continue;
                    }

                    bool sold = false;
                    for (const auto& fx : league.getFixtures()) {
                        std::string a = fx->getFixtureId();
                        std::string b = fxId;
                        std::transform(a.begin(), a.end(), a.begin(), ::toupper);
                        std::transform(b.begin(), b.end(), b.begin(), ::toupper);

                        if (a == b) {
                            if (fx->getTickets().empty()) {
                                std::cout << "\n[NOTICE] No tickets have been generated for fixture " << fx->getFixtureId() << " yet!\n";
                                std::cout << "Please choose Option 4 (Generate Tickets for a Fixture) first.\n";
                                sold = true;
                                break;
                            }
                            Ticket* t = fx->sellTicket(cat, *spectator);
                            // spectators first, so a saved ticket never refers to a missing buyer
                            DataPersistence::saveSpectators(league, "kpfl_spectators.txt");
                            DataPersistence::saveTickets(league, "kpfl_tickets.txt");
                            std::cout << "\n[SUCCESS] Ticket sold successfully!\n";
                            t->displayInfo();
                            sold = true;
                            break;
                        }
                    }
                    if (!sold) std::cout << "\n[ERROR] Fixture '" << fxId << "' was not found or the category is sold out!\n";
                    pausePrompt();
                } else if (ch == 6) {
                    clearScreen();
                    std::cout << "=== REGISTER SPECTATOR ===\n\n";

                    std::cout << "Available Teams in League (for fan preference):\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - [" << t->getRegNumber() << "] " << t->getName()
                                  << " (" << t->getHomeCity() << ")\n";
                    }
                    std::cout << "  - Or type 'Neutral' / leave blank if neutral fan\n\n";

                    std::string spId, name, gender, email, phone, prefTeam;
                    int age;
                    std::cout << "Enter New Spectator ID (e.g. SP03, SP04): "; std::cin >> spId;
                    clearInput();
                    std::cout << "Enter Full Name: "; std::getline(std::cin, name);
                    std::cout << "Enter Age: "; std::cin >> age;
                    std::cout << "Enter Gender (M/F): "; std::cin >> gender;
                    clearInput();
                    std::cout << "Enter Email Address: "; std::getline(std::cin, email);
                    std::cout << "Enter Phone Number: "; std::getline(std::cin, phone);
                    std::cout << "Enter Preferred Team Name (from list above): "; std::getline(std::cin, prefTeam);

                    auto sp = new Spectator(spId, name, age, gender, email, phone, prefTeam);
                    league.addSpectator(sp);
                    DataPersistence::saveSpectators(league, "kpfl_spectators.txt");
                    std::cout << "\n[SUCCESS] Spectator " << name << " registered successfully!\n";
                    pausePrompt();
                } else if (ch == 7) {
                    clearScreen();
                    for (const auto* sp : league.getSpectators()) sp->displayInfo();
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
                    std::cout << "=== ADD NEW MERCHANDISE ===\n\n";

                    std::cout << "Available Teams to link merchandise to:\n";
                    for (const auto* t : league.getTeams()) {
                        std::cout << "  - [" << t->getRegNumber() << "] " << t->getName() << "\n";
                    }
                    std::cout << "  - Or type 'League' for general KPFL merchandise\n\n";

                    std::string pId, name, size, team;
                    double price;
                    int stock;
                    std::cout << "Enter Product ID (e.g. M03, M04): "; std::cin >> pId;
                    clearInput();
                    std::cout << "Enter Item Name (e.g. Cavalier SC Home Jersey): "; std::getline(std::cin, name);
                    std::cout << "Enter Price in JMD (e.g. 8500): "; std::cin >> price;
                    std::cout << "Enter Stock Quantity (e.g. 30): "; std::cin >> stock;
                    std::cout << "Enter Size (S / M / L / XL / Standard): "; std::cin >> size;
                    clearInput();
                    std::cout << "Enter Team Reg No (from list above, or 'League'): "; std::getline(std::cin, team);

                    shop.addProduct(std::make_unique<Merchandise>(pId, name, price, stock, size, team));
                    DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    std::cout << "\n[SUCCESS] Added merchandise '" << name << "' to shop inventory!\n";
                    pausePrompt();
                } else if (ch == 3) {
                    clearScreen();
                    std::cout << "=== ADD NEW FOOD / BEVERAGE ===\n\n";
                    std::string pId, name;
                    double price;
                    int stock, isBev;
                    std::cout << "Enter Product ID (e.g. F04): "; std::cin >> pId;
                    clearInput();
                    std::cout << "Enter Item Name: "; std::getline(std::cin, name);
                    std::cout << "Enter Price (JMD): "; std::cin >> price;
                    std::cout << "Enter Stock Quantity: "; std::cin >> stock;
                    std::cout << "Is it a beverage? (1 for Yes, 0 for Food): "; std::cin >> isBev;
                    clearInput();

                    shop.addProduct(std::make_unique<FoodItem>(pId, name, price, stock, isBev == 1));
                    DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    std::cout << "\n[SUCCESS] Added food/beverage item to shop!\n";
                    pausePrompt();
                } else if (ch == 4) {
                    clearScreen();
                    std::cout << "=== PROCESS CUSTOMER SALE ===\n\n";
                    shop.displayInventory();

                    std::string custName;
                    std::cout << "Enter Customer Name: "; std::getline(std::cin, custName);

                    std::vector<std::pair<std::string, int>> cart;
                    while (true) {
                        std::string prodId;
                        int qty;
                        std::cout << "Enter Product ID to purchase (or 'done' to finish): "; std::cin >> prodId;
                        if (prodId == "done") break;
                        std::cout << "Enter Quantity: "; std::cin >> qty;
                        cart.push_back({prodId, qty});
                    }

                    if (!cart.empty()) {
                        double cash;
                        std::cout << "\nEnter Cash Tendered (JMD): "; std::cin >> cash;
                        clearInput();

                        clearScreen();
                        Receipt r = shop.processSale(custName, cart, cash);
                        StadiumShop::printReceipt(r);
                        DataPersistence::saveTransactions(shop, "kpfl_shop_receipts.txt");
                        DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
                    } else {
                        clearInput();
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
            } catch (const std::exception& e) {
                std::cout << "\n[SHOP/FINANCE ERROR] " << e.what() << "\n";
                pausePrompt();
            }
        }
    }
};
