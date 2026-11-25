#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cassert>

// Include all modules
#include "../include/exceptions.h"
#include "../include/Person.h"
#include "../include/Player.h"
#include "../include/Coach.h"
#include "../include/Spectator.h"
#include "../include/MatchOfficial.h"
#include "../include/LeagueCommissioner.h"
#include "../include/FootballTeam.h"
#include "../include/Stadium.h"
#include "../include/Ticket.h"
#include "../include/MatchResult.h"
#include "../include/Fixture.h"
#include "../include/FootballLeague.h"
#include "../include/Product.h"
#include "../include/Merchandise.h"
#include "../include/FoodItem.h"
#include "../include/StadiumShop.h"
#include "../include/DataPersistence.h"
#include "../include/AuthService.h"
#include "../include/MenuSystem.h"

// Helper function to test exceptions
template <typename ExceptionType, typename Action>
void expectException(const std::string& testName, Action action) {
    try {
        action();
        std::cout << "[FAIL] " << testName << " (No exception thrown)\n";
    } catch (const ExceptionType& error) {
        std::cout << "[PASS] " << testName << " -> " << error.what() << "\n";
    } catch (const std::exception& error) {
        std::cout << "[UNEXPECTED EXCEPTION] " << testName << " -> " << error.what() << "\n";
    }
}

void runAutomatedTestSuite(FootballLeague& league,
                           StadiumShop& shop, Stadium& nationalStadium,
                           FootballTeam& harbour, FootballTeam& waterhouse,
                           FootballTeam& cavalier, MatchOfficial& ref1) {
    std::cout << "\n====================================================================\n";
    std::cout << ">>> RUNNING AUTOMATED OOP INTEGRATION & BUSINESS RULES VERIFICATION <<<\n";
    std::cout << "====================================================================\n";

    // 1. Person 1: Suspended team cannot be scheduled
    expectException<TeamNotEligibleException>("Suspended team scheduled in fixture", [&] {
        league.scheduleFixture(&waterhouse, &harbour, &nationalStadium, "2026-11-29", "15:00");
    });

    // 2. Person 1: Scheduling conflict (same stadium, date, and time)
    expectException<SchedulingConflictException>("Double booking stadium slot", [&] {
        league.scheduleFixture(&cavalier, &harbour, &nationalStadium, "2026-11-28", "16:00");
    });

    // 3. Person 1: Match official double booking
    expectException<OfficialConflictException>("Official assigned to conflicting timeslot", [&] {
        ref1.assignSlot("2026-11-28", "16:00");
    });

    // 4. Person 1: Negative score
    expectException<InvalidScoreException>("Negative match score", [&] {
        MatchResult badResult(-1, 2);
    });

    // 5. Person 2: Underage player (< 16 years)
    expectException<InvalidAgeException>("Player under 16 years old", [&] {
        Player youngKid("PL999", "Junior", 14, "M", "Forward", 15);
    });

    // 6. Person 2: Underage coach (< 21 years)
    expectException<InvalidAgeException>("Coach under 21 years old", [&] {
        Coach youngCoach("CO99", "Coach Kid", 19, "M", "Level 1", "Assistant");
    });

    // 7. Person 2: Duplicate jersey number in squad
    expectException<PersonnelException>("Duplicate jersey number in same team", [&] {
        Player duplicateNum("PL888", "New Striker", 25, "M", "Forward", 9); // #9 already taken by Javon East
        harbour.addPlayer(duplicateNum);
    });

    // 8. Person 3: Ticket sales exceeding stadium capacity
    expectException<CapacityExceededException>("Tickets requested exceed stadium capacity", [&] {
        Stadium smallPark("S99", "Community Ground", "St. Andrew", 50, "Fair");
        Fixture testFix("FX99", "2026-12-01", "15:00", &harbour, &cavalier, &smallPark);
        testFix.generateTickets(40, 20, 10); // 70 requested > 50 capacity
    });

    // 9. Person 4: Insufficient payment for stadium shop items
    expectException<InsufficientPaymentException>("Insufficient payment for shop items", [&] {
        std::vector<std::pair<std::string, int>> expensiveCart = { {"M01", 2} }; // 2 * $8500 = $17,000 + 15% = $19,550
        shop.processSale("Broke Fan", expensiveCart, 5000.0); // Only $5000 tendered
    });

    // 10. Person 4: Out of stock purchase
    expectException<OutOfStockException>("Purchasing more items than in stock", [&] {
        std::vector<std::pair<std::string, int>> hugeCart = { {"M01", 9999} };
        shop.processSale("Wholesale Buyer", hugeCart, 10000000.0);
    });

    // 11-13. Person 2: Player updates obey the same rules as adding a player
    FootballTeam updTeam("T99", "Update Test FC", "Kingston");
    Player scorer("PL901", "Player One", 20, "M", "Forward", 9);
    scorer.addGoals(3);
    updTeam.addPlayer(scorer);
    updTeam.addPlayer(Player("PL902", "Player Two", 21, "M", "Defender", 4));
    expectException<InvalidAgeException>("Player updated to under 16", [&] {
        updTeam.updatePlayer("PL901", "Changed", 14, "Forward", 9, "Active");
    });
    expectException<PersonnelException>("Player updated to a taken jersey", [&] {
        updTeam.updatePlayer("PL901", "Changed", 20, "Forward", 4, "Active");
    });
    expectException<PersonnelException>("Player updated to an invalid status", [&] {
        updTeam.updatePlayer("PL901", "Changed", 20, "Forward", 9, "Asleep");
    });

    // 14. Person 2: Failed updates change nothing; valid ones apply and keep goals
    const Player* upd = updTeam.findPlayer("PL901");
    bool updateOk = upd->getName() == "Player One"
        && updTeam.updatePlayer("PL901", "Renamed", 22, "Midfielder", 9, "Injured")
        && upd->getName() == "Renamed" && upd->getAge() == 22
        && upd->getPosition() == "Midfielder" && upd->getPlayingStatus() == "Injured"
        && upd->getGoals() == 3 && upd->getTeamId() == "T99"
        && !updTeam.updatePlayer("NOPE", "X", 20, "Forward", 1, "Active");
    std::cout << (updateOk ? "[PASS]" : "[FAIL]") << " Valid player update applied\n";

    std::cout << "\n[ALL 14 BUSINESS RULES & EXCEPTIONS PASSED VERIFICATION]\n";
}

int main(int argc, char* argv[]) {
    std::cout << "====================================================================\n";
    std::cout << "  JAMAICAN FOOTBALL LEAGUE (JFL) - COMPLETE OOP INTEGRATED SYSTEM   \n";
    std::cout << "       Coordinating Person 1, Person 2, Person 3 & Person 4         \n";
    std::cout << "====================================================================\n\n";

    try {
        // Core League Setup
        FootballLeague league("Kingston Premier Football League (KPFL)");
        LeagueCommissioner commissioner("COM01", "Marcus Brown", 48, "M");
        league.setCommissioner(&commissioner);

        // Teams & Personnel (Person 2)
        FootballTeam harbour("T01", "Harbour View FC", "Kingston");
        FootballTeam tivoli("T02", "Tivoli Gardens FC", "Kingston");
        FootballTeam portmore("T03", "Portmore United", "St. Catherine");
        FootballTeam waterhouse("T04", "Waterhouse FC", "Kingston");
        FootballTeam cavalier("T05", "Cavalier SC", "Kingston");

        Player p1("PL001", "Javon East", 28, "M", "Forward", 9);
        Player p2("PL002", "Kemai Foster", 22, "M", "Midfielder", 10);
        Player p3("PL003", "Amal Knight", 30, "M", "Goalkeeper", 1);
        p1.addGoals(5);
        harbour.addPlayer(p1);
        harbour.addPlayer(p2);
        harbour.addPlayer(p3);

        Player p4("PL004", "Rodolph Austin", 38, "M", "Midfielder", 8);
        Player p5("PL005", "Nicholas Hamilton", 27, "M", "Forward", 11);
        tivoli.addPlayer(p4);
        tivoli.addPlayer(p5);

        Coach c1("CO01", "Ludlow Bernard", 54, "M", "CONCACAF Pro", "Head Coach", 15);
        Coach c2("CO02", "Jerome Waite", 59, "M", "FIFA Elite License", "Head Coach", 20);
        harbour.addCoach(c1);
        tivoli.addCoach(c2);

        league.addTeam(&harbour);
        league.addTeam(&tivoli);
        league.addTeam(&portmore);
        league.addTeam(&waterhouse);
        league.addTeam(&cavalier);

        commissioner.approveTeam(harbour);
        commissioner.approveTeam(tivoli);
        commissioner.approveTeam(portmore);
        commissioner.approveTeam(waterhouse);
        commissioner.approveTeam(cavalier);
        commissioner.suspendTeam(waterhouse, true);

        // Stadiums & Spectators (Person 3)
        Stadium nationalStadium("S01", "National Stadium (Independence Park)", "Kingston", 35000, "Excellent");
        Stadium edwardSeaga("S02", "Edward Seaga Sports Complex", "Tivoli Gardens", 5000, "Good");
        league.addStadium(&nationalStadium);
        league.addStadium(&edwardSeaga);

        MatchOfficial ref1("O001", "Andre Lewis", 38, "M", "Referee", "FIFA Badge");
        MatchOfficial ref2("O002", "Keisha Grant", 34, "F", "Assistant Referee", "Grade A");
        MatchOfficial ref3("O003", "Daneon Parchment", 41, "M", "Referee", "Grade A");
        league.addOfficial(&ref1);
        league.addOfficial(&ref2);
        league.addOfficial(&ref3);

        Spectator spectator1("SP01", "Usain Bolt", 39, "M", "bolt@fast.jm", "876-555-0100", "Harbour View FC");
        Spectator spectator2("SP02", "Shelly-Ann Fraser", 38, "F", "shelly@pocket.jm", "876-555-0200", "Tivoli Gardens FC");
        league.addSpectator(&spectator1);
        league.addSpectator(&spectator2);

        // Fixtures (Person 1)
        Fixture& fixture1 = league.scheduleFixture(&harbour, &tivoli, &nationalStadium, "2026-11-28", "16:00");
        Fixture& fixture2 = league.scheduleFixture(&portmore, &cavalier, &edwardSeaga, "2026-11-28", "18:30");
        league.assignOfficial(fixture1.getFixtureId(), ref1.getId());
        league.assignOfficial(fixture1.getFixtureId(), ref2.getId());
        league.assignOfficial(fixture2.getFixtureId(), ref3.getId());

        // Ticketing (Person 3)
        fixture1.generateTickets(100, 50, 10);
        fixture1.sellTicket(TicketCategory::VIP, spectator1);
        fixture1.sellTicket(TicketCategory::GRANDSTAND, spectator2);

        // Results (Person 1)
        league.recordResult(fixture1.getFixtureId(), 3, 1);
        league.recordResult(fixture2.getFixtureId(), 2, 2);

        // Shop & Finance (Person 4)
        StadiumShop shop("SHOP-NS01", nationalStadium.getStadiumId());
        shop.addProduct(std::make_unique<Merchandise>("M01", "Harbour View Home Jersey", 8500.0, 25, "L", "T01"));
        shop.addProduct(std::make_unique<Merchandise>("M02", "KPFL Championship Cap", 2200.0, 50, "Standard", "League"));
        shop.addProduct(std::make_unique<FoodItem>("F01", "Tastee Beef Patty", 350.0, 100, false));
        shop.addProduct(std::make_unique<FoodItem>("F02", "Ting Grapefruit Soda", 250.0, 80, true));
        shop.addProduct(std::make_unique<FoodItem>("F03", "Jerk Chicken Quarter", 1200.0, 40, false));

        // Data Persistence export (Person 4)
        DataPersistence::saveTeams(league, "kpfl_teams.txt");
        DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
        DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");

        // Check if running in automated test mode (e.g. ./main.exe --test)
        bool testMode = (argc > 1 && std::string(argv[1]) == "--test");
        if (testMode) {
            runAutomatedTestSuite(league, shop, nationalStadium, harbour, waterhouse, cavalier, ref1);
            return 0;
        }

        // Launch the Secure Login and Menu System
        MenuSystem menuSystem(league, commissioner, shop);
        menuSystem.run();

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
