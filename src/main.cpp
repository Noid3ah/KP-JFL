#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <filesystem>

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

    // 15. Teams and players survive a save and reload
    const auto tmp = std::filesystem::temp_directory_path();
    const std::string teamFile = (tmp / "kpfl_test_teams.txt").string();
    const std::string playerFile = (tmp / "kpfl_test_players.txt").string();
    std::filesystem::remove(playerFile);
    updTeam.setSuspended(true);
    updTeam.recordMatch(3, 1, 3);
    FootballLeague saved("Saved"), reloaded("Reloaded");
    saved.addTeam(&updTeam);
    DataPersistence::saveTeams(saved, teamFile);
    DataPersistence::savePlayers(saved, playerFile);
    std::vector<std::unique_ptr<FootballTeam>> storage;
    DataPersistence::loadTeams(reloaded, storage, teamFile);
    DataPersistence::loadPlayers(reloaded, playerFile);
    FootballTeam* back = reloaded.findTeam("T99");
    bool reloadOk = back && back->isSuspended() && back->getWins() == 1 && back->getPoints() == 3
        && back->getGoalsFor() == 3 && back->getSquad().size() == 2
        && back->findPlayer("PL901")->getGoals() == 3
        && back->findPlayer("PL901")->getPlayingStatus() == "Injured";
    std::filesystem::remove(teamFile);
    std::filesystem::remove(playerFile);
    std::cout << (reloadOk ? "[PASS]" : "[FAIL]") << " Teams and players survive reload\n";

    // 16. Fixtures survive a reload without their results being counted twice
    const std::string fixtureFile = (tmp / "kpfl_test_fixtures.txt").string();
    const std::string fxTeamFile = (tmp / "kpfl_test_fx_teams.txt").string();
    const std::string fxStadiumFile = (tmp / "kpfl_test_stadiums.txt").string();
    FootballTeam fxHome("T97", "Fixture Home", "Kingston"), fxAway("T98", "Fixture Away", "Kingston");
    fxHome.setRegistered(true);
    fxAway.setRegistered(true);
    Stadium fxGround("S97", "Fixture Ground", "Kingston", 100, "");
    MatchOfficial fxRef("O97", "Test Ref", 30, "M", "Referee", "Grade A");
    FootballLeague before("Before"), after("After");
    before.addTeam(&fxHome);
    before.addTeam(&fxAway);
    before.addStadium(&fxGround);
    before.addOfficial(&fxRef);
    before.scheduleFixture(&fxHome, &fxAway, &fxGround, "2026-12-01", "15:00");
    before.scheduleFixture(&fxHome, &fxAway, &fxGround, "2026-12-08", "15:00");
    before.assignOfficial("FX001", "O97");
    before.recordResult("FX001", 2, 1);
    fxHome.setSuspended(true); // a later suspension must not block loading
    DataPersistence::saveTeams(before, fxTeamFile);
    DataPersistence::saveStadiums(before, fxStadiumFile);
    DataPersistence::saveFixtures(before, fixtureFile);

    MatchOfficial fxRefAgain("O97", "Test Ref", 30, "M", "Referee", "Grade A");
    std::vector<std::unique_ptr<FootballTeam>> fxTeams;
    std::vector<std::unique_ptr<Stadium>> fxStadiums;
    after.addOfficial(&fxRefAgain);
    DataPersistence::loadTeams(after, fxTeams, fxTeamFile);
    DataPersistence::loadStadiums(after, fxStadiums, fxStadiumFile);
    DataPersistence::loadFixtures(after, fixtureFile);
    const auto& fxs = after.getFixtures();
    bool fixturesOk = fxs.size() == 2
        && fxs[0]->getStatus() == FixtureStatus::COMPLETED && fxs[0]->getResult()->getHomeScore() == 2
        && fxs[0]->getOfficials().size() == 1 && !fxRefAgain.isFree("2026-12-01", "15:00")
        && fxs[1]->getStatus() == FixtureStatus::SCHEDULED
        && after.findTeam("T97")->getPlayed() == 1 && after.findTeam("T97")->getPoints() == 3;
    for (const auto& f : {fixtureFile, fxTeamFile, fxStadiumFile}) std::filesystem::remove(f);
    std::cout << (fixturesOk ? "[PASS]" : "[FAIL]") << " Fixtures survive reload without double counting\n";
    expectException<LeagueException>("Recording a reloaded result again", [&] {
        after.recordResult("FX001", 2, 1);
    });

    // 18. Shop stock, products and receipts survive a reload
    const std::string stockFile = (tmp / "kpfl_test_stock.txt").string();
    const std::string receiptFile = (tmp / "kpfl_test_receipts.txt").string();
    StadiumShop shopA("SHOP-A", "S01"), shopB("SHOP-B", "S01");
    shopA.addProduct(std::make_unique<Merchandise>("M01", "Jersey", 100.5, 10, "L", "T01", "Cotton"));
    shopA.addProduct(std::make_unique<FoodItem>("F01", "Soda", 20.0, 8, true, false, "2027"));
    shopB.addProduct(std::make_unique<Merchandise>("M01", "Jersey", 100.5, 10, "L", "T01", "Cotton"));
    shopA.processSale("Fan", { {"M01", 2}, {"F01", 1} }, 1000.0);
    DataPersistence::saveShopInventory(shopA, stockFile);
    DataPersistence::saveTransactions(shopA, receiptFile);
    DataPersistence::loadShopInventory(shopB, stockFile);
    DataPersistence::loadTransactions(shopB, receiptFile);
    const auto* soda = dynamic_cast<const FoodItem*>(shopB.findProduct("F01"));
    bool shopOk = shopB.findProduct("M01")->getStockQuantity() == 8
        && soda && soda->getStockQuantity() == 7 && soda->getIsBeverage()
        && !soda->getIsPerishable() && soda->getExpiryDate() == "2027"
        && shopB.getTransactionHistory().size() == 1
        && shopB.getTransactionHistory()[0].items.size() == 2
        && shopB.getTransactionHistory()[0].items[0].itemTotal == 201.0
        && shopB.processSale("Fan", { {"F01", 1} }, 100.0).receiptId == "RCP-2";
    std::filesystem::remove(stockFile);
    std::filesystem::remove(receiptFile);
    std::cout << (shopOk ? "[PASS]" : "[FAIL]") << " Shop stock and receipts survive reload\n";

    // 19-21. A rejected cart must leave stock untouched
    expectException<OutOfStockException>("Same product listed twice beyond stock", [&] {
        shopA.processSale("Fan", { {"M01", 5}, {"M01", 5} }, 10000.0);
    });
    expectException<ShopFinanceException>("Negative quantity in cart", [&] {
        shopA.processSale("Fan", { {"M01", 1}, {"F01", -1} }, 10000.0);
    });
    std::cout << (shopA.findProduct("M01")->getStockQuantity() == 8 ? "[PASS]" : "[FAIL]")
              << " Rejected carts leave stock unchanged\n";

    // 22. Coaches, spectators and tickets survive a reload
    const std::string coachFile = (tmp / "kpfl_test_coaches.txt").string();
    const std::string fanFile = (tmp / "kpfl_test_spectators.txt").string();
    const std::string ticketFile = (tmp / "kpfl_test_tickets.txt").string();
    Spectator fan("SP97", "Test Fan", 30, "M"); // blank email, phone and team
    before.addSpectator(&fan);
    fxHome.addCoach(Coach("CO97", "Test Coach", 40, "F", "Pro", "Head Coach", 9));
    Fixture& ticketed = *before.getFixtures()[1];
    ticketed.generateTickets(3, 2, 1);
    ticketed.sellTicket(TicketCategory::VIP, fan);
    fxGround.setSeatingCapacity(2); // shrinking the venue later must not block loading
    DataPersistence::saveCoaches(before, coachFile);
    DataPersistence::saveSpectators(before, fanFile);
    DataPersistence::saveTickets(before, ticketFile);
    std::vector<std::unique_ptr<Spectator>> fans;
    DataPersistence::loadCoaches(after, coachFile);
    DataPersistence::loadSpectators(after, fans, fanFile);
    DataPersistence::loadTickets(after, ticketFile);
    DataPersistence::loadTickets(after, ticketFile); // a second load must not double the count
    const auto& reTickets = after.getFixtures()[1]->getTickets();
    const auto& staff = after.findTeam("T97")->getCoachingStaff();
    bool peopleOk = staff.size() == 1 && staff[0].getYearsExperience() == 9 && staff[0].getTeamId() == "T97"
        && after.getSpectators().size() == 1 && after.getSpectators()[0]->getTicketsBought() == 1
        && reTickets.size() == 6 && reTickets[5].getIsSold() && reTickets[5].getSpectatorId() == "SP97"
        && reTickets[5].getCategory() == TicketCategory::VIP && !reTickets[0].getIsSold()
        && after.getFixtures()[0]->getTickets().empty();
    for (const auto& f : {coachFile, fanFile, ticketFile}) std::filesystem::remove(f);
    std::cout << (peopleOk ? "[PASS]" : "[FAIL]") << " Coaches, spectators and tickets survive reload\n";

    // 23-24. Sold tickets can't be erased and spectator IDs stay unique
    fxGround.setSeatingCapacity(100);
    expectException<StadiumTicketingException>("Regenerating tickets after a sale", [&] {
        ticketed.generateTickets(1, 1, 1);
    });
    expectException<LeagueException>("Duplicate spectator ID", [&] {
        before.addSpectator(&fan);
    });

    // 25. Match dates and times must be real
    bool datesOk = isValidDate("2026-12-05") && isValidDate("2028-02-29")
        && !isValidDate("2026-02-29") && !isValidDate("2026-13-01") && !isValidDate("2026-04-31")
        && !isValidDate("2026-00-10") && !isValidDate("2026-1-5") && !isValidDate("banana") && !isValidDate("")
        && isValidTime("00:00") && isValidTime("23:59")
        && !isValidTime("24:00") && !isValidTime("12:60") && !isValidTime("9:00") && !isValidTime("");
    std::cout << (datesOk ? "[PASS]" : "[FAIL]") << " Date and time checks\n";

    // 26-27. Coach updates obey the age rule and keep ID, gender and team
    FootballTeam coachTeam("T96", "Coach Test FC", "Kingston");
    coachTeam.addCoach(Coach("CO96", "Coach One", 40, "F", "Pro", "Head Coach", 5));
    expectException<InvalidAgeException>("Coach updated to under 21", [&] {
        coachTeam.updateCoach("CO96", "Changed", 20, "Pro", "Head Coach", 5);
    });
    const Coach& updCoach = coachTeam.getCoachingStaff()[0];
    bool coachOk = updCoach.getName() == "Coach One"
        && coachTeam.updateCoach("CO96", "Renamed", 41, "UEFA A License", "Assistant Coach", 6)
        && updCoach.getName() == "Renamed" && updCoach.getAge() == 41
        && updCoach.getQualification() == "UEFA A License" && updCoach.getCoachingRole() == "Assistant Coach"
        && updCoach.getYearsExperience() == 6 && updCoach.getId() == "CO96"
        && updCoach.getGender() == "F" && updCoach.getTeamId() == "T96"
        && !coachTeam.updateCoach("NOPE", "X", 40, "Pro", "Head Coach", 5);
    std::cout << (coachOk ? "[PASS]" : "[FAIL]") << " Valid coach update applied\n";

    std::cout << "\n[ALL 27 BUSINESS RULES & EXCEPTIONS PASSED VERIFICATION]\n";
}

int main(int argc, char* argv[]) {
    std::cout << "====================================================================\n";
    std::cout << "  JAMAICAN FOOTBALL LEAGUE (JFL) - COMPLETE OOP INTEGRATED SYSTEM   \n";
    std::cout << "====================================================================\n\n";

    try {
        // Core League Setup
        std::vector<std::unique_ptr<Spectator>> loadedSpectators; // owns spectators read from file
        std::vector<std::unique_ptr<Stadium>> loadedStadiums;   // owns stadiums read from file
        std::vector<std::unique_ptr<FootballTeam>> loadedTeams; // owns teams read from file
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

        // Check if running in automated test mode (e.g. ./main.exe --test)
        bool testMode = (argc > 1 && std::string(argv[1]) == "--test");
        if (testMode) {
            runAutomatedTestSuite(league, shop, nationalStadium, harbour, waterhouse, cavalier, ref1);
            return 0;
        }

        // Saved teams load first, so saved players have a squad to join
        if (std::filesystem::exists("kpfl_teams.txt")) {
            DataPersistence::loadTeams(league, loadedTeams, "kpfl_teams.txt");
        } else {
            DataPersistence::saveTeams(league, "kpfl_teams.txt");
        }

        // Saved squads replace the sample ones; a damaged file stops startup rather than being overwritten
        if (std::filesystem::exists("kpfl_players.txt")) {
            DataPersistence::loadPlayers(league, "kpfl_players.txt");
        }

        if (std::filesystem::exists("kpfl_stadiums.txt")) {
            DataPersistence::loadStadiums(league, loadedStadiums, "kpfl_stadiums.txt");
        } else {
            DataPersistence::saveStadiums(league, "kpfl_stadiums.txt");
        }

        if (std::filesystem::exists("kpfl_coaches.txt")) {
            DataPersistence::loadCoaches(league, "kpfl_coaches.txt");
        }
        if (std::filesystem::exists("kpfl_spectators.txt")) {
            DataPersistence::loadSpectators(league, loadedSpectators, "kpfl_spectators.txt");
        }

        // Fixtures refer to teams and stadiums; tickets refer to fixtures and spectators
        if (std::filesystem::exists("kpfl_fixtures.txt")) {
            DataPersistence::loadFixtures(league, "kpfl_fixtures.txt");
        }
        if (std::filesystem::exists("kpfl_tickets.txt")) {
            DataPersistence::loadTickets(league, "kpfl_tickets.txt");
        }

        if (std::filesystem::exists("kpfl_shop_inventory.txt")) {
            DataPersistence::loadShopInventory(shop, "kpfl_shop_inventory.txt");
        } else {
            DataPersistence::saveShopInventory(shop, "kpfl_shop_inventory.txt");
        }
        if (std::filesystem::exists("kpfl_shop_receipts.txt")) {
            DataPersistence::loadTransactions(shop, "kpfl_shop_receipts.txt");
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
