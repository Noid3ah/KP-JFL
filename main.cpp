// Demo program for Person 1's league operations.
#include <iostream>
#include <string>

#include "FootballLeague.h"

// Runs one test and reports whether the expected exception was caught.
template <typename ExceptionType, typename Action>
void expectException(const std::string &testName, Action action) {
  try {
    action();
    std::cout << "[expectException] [FAIL] " << testName
              << " (no exception was thrown)\n";
  } catch (const ExceptionType &error) {
    std::cout << "[expectException] [PASS] " << testName << " -> "
              << error.what() << "\n";
  }
}

// Holds the sample teams, stadiums, officials, and league used by the demo.
struct LeagueDemo {
  LeagueCommissioner commissioner{"C001", "Marcus Brown", 45, "M"};

  FootballTeam harbour{"T01", "Harbour View"};
  FootballTeam tivoli{"T02", "Tivoli Gardens"};
  FootballTeam portmore{"T03", "Portmore United"};
  FootballTeam waterhouse{"T04", "Waterhouse"};
  FootballTeam humbleLion{"T05", "Humble Lion"};
  FootballTeam cavalier{"T06", "Cavalier"};

  Stadium independencePark{"S01", "Independence Park"};
  Stadium spauldingArena{"S02", "Anthony Spaulding"};

  MatchOfficial andreLewis{"O001", "Andre Lewis", 38,
                           "M",    "Referee",     "Grade A"};
  MatchOfficial keishaGrant{"O002", "Keisha Grant", 34,
                            "F",    "Referee",      "Grade B"};

  // Keep the league after the objects it refers to so they stay alive
  // throughout the demo.
  FootballLeague league{"Kingston Premier Football League"};
  Fixture *firstFixture = nullptr;

  LeagueDemo() {
    std::cout << "[LeagueDemo constructor] Setting up sample league data.\n";

    league.setCommissioner(&commissioner);

    league.addTeam(&harbour);
    league.addTeam(&tivoli);
    league.addTeam(&portmore);
    league.addTeam(&waterhouse);
    league.addTeam(&humbleLion);
    league.addTeam(&cavalier);

    league.addStadium(&independencePark);
    league.addStadium(&spauldingArena);

    league.addOfficial(&andreLewis);
    league.addOfficial(&keishaGrant);

    commissioner.approveTeam(harbour);
    commissioner.approveTeam(tivoli);
    commissioner.approveTeam(portmore);
    commissioner.approveTeam(waterhouse);
    commissioner.approveTeam(cavalier);

    // These two teams let the demo test suspended and unregistered teams.
    waterhouse.setSuspended(true);
  }
};

void showScheduledFixtures(LeagueDemo &demo) {
  std::cout << "\n=== showScheduledFixtures: Schedule sample matches ===\n";

  demo.firstFixture = &demo.league.scheduleFixture(&demo.harbour, &demo.tivoli,
                                                   &demo.independencePark,
                                                   "2026-11-14", "15:00");

  demo.league.scheduleFixture(&demo.portmore, &demo.cavalier,
                              &demo.spauldingArena, "2026-11-14", "15:00");

  demo.league.assignOfficial("FX001", "O001");
  demo.commissioner.inspectMatch(*demo.firstFixture);
  demo.league.displayFixtures();
}

void checkInvalidCases(LeagueDemo &demo) {
  std::cout
      << "\n=== checkInvalidCases: Test invalid and conflicting actions ===\n";

  // Suspended and unregistered teams cannot be scheduled.
  expectException<TeamNotEligibleException>("Suspended team", [&demo] {
    demo.league.scheduleFixture(&demo.waterhouse, &demo.tivoli,
                                &demo.independencePark, "2026-11-21", "15:00");
  });

  expectException<TeamNotEligibleException>("Unregistered team", [&demo] {
    demo.league.scheduleFixture(&demo.humbleLion, &demo.tivoli,
                                &demo.independencePark, "2026-11-21", "15:00");
  });

  // A team cannot play two matches at the same time.
  expectException<SchedulingConflictException>("Team double-booked", [&demo] {
    demo.league.scheduleFixture(&demo.harbour, &demo.cavalier,
                                &demo.independencePark, "2026-11-14", "15:00");
  });

  // An official already assigned to a match cannot work another at that time.
  expectException<OfficialConflictException>("Official double-booked", [&demo] {
    demo.league.assignOfficial("FX002", "O001");
  });

  // Scores cannot be negative, and a fixture needs a result before completion.
  expectException<InvalidScoreException>(
      "Negative score", [&demo] { demo.league.recordResult("FX001", -1, 2); });

  expectException<ResultRequiredException>(
      "Complete without result", [&demo] { demo.firstFixture->complete(); });
}

void recordResultsAndShowStandings(LeagueDemo &demo) {
  std::cout << "\n=== recordResultsAndShowStandings: Record scores and show "
               "table ===\n";

  demo.league.recordResult("FX001", 2, 1);
  demo.league.recordResult("FX002", 0, 0); // A 0-0 score is a draw.

  // A completed fixture cannot be given a second result.
  expectException<LeagueException>("Result recorded twice", [&demo] {
    demo.league.recordResult("FX001", 3, 3);
  });

  demo.league.displayFixtures();
  demo.league.displayStandings();
}

int main() {
  std::cout << "=== main: Start league operations demo ===\n";
  LeagueDemo demo;

  showScheduledFixtures(demo);
  checkInvalidCases(demo);
  recordResultsAndShowStandings(demo);

  std::cout << "\n=== main: Demo finished ===\n";
  return 0;
}
