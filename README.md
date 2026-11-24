# KPFL

## Project overview

The Kingston Premier Football League (KPFL) requires a group of three to five members to create an object-
oriented solution to manage their football competition. The KPFL consists of football teams, players, coaches, match
officials, stadiums, fixtures, ticket sales, merchandise, and match-day operations.
The minimum submission should be a functional application with persistent data storage using sequential
access files, random access files, or both. You may include a graphical user interface (GUI) as the interface for your
solution; however, it should only complement the functionality of your solution, not replace the required object-
oriented design, validation, exception handling, and testing.

## Scenario

The scenario is as follows: In the Parish of Kingston, the Kingston Premier Football League (KPFL) runs a
data-driven football competition each year. The competition organizers allow all teams to request permission to use a
stadium for practice before each match. Each Team has a name, parish, registration number, squad size, league status,
players, and coaching staff.
Each Stadium has a stadium ID, name, location, seating capacity, field condition, ticket price, and a record of
the teams assigned to the current fixture. The Football League manages teams, stadiums, fixtures, results, and league
standings. A League Commissioner approves registrations, inspects match preparations, and coordinates the
assignment of Match Officials to each match.
The League Commissioner, Player, Coach, Match Official, and Spectator are all specialised types of Person.
Each Person has an identification number, name, age, and gender. On match days, spectators may purchase tickets
and products from the Stadium Shop, including merchandise, food, and beverages. All prices include a 15% general
consumption tax (GCT).

## Core functional requirements

1. Register, update, search for, and display football teams.
2. Add, update, and remove players and coaches associated with a team.
3. Register stadiums and record seating capacity and field condition.
4. Create fixtures and assign teams, stadiums, dates, and times.
5. Register and assign match officials without scheduling conflicts.
6. Record match results and calculate league points and standings.
7. Sell tickets while monitoring stadium capacity.
8. Manage products and complete stadium-shop transactions.
9. Calculate subtotal, tax, total payment, and change.
10. Save and reload system data using files.
11. Produce at least three useful management reports.
12. Use exceptions to handle invalid data and prohibited operations.
