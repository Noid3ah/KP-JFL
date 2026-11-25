# Jamaican Football League (JFL) — UML Class Diagram & Architecture

This document provides the complete UML Class Diagram, inheritance structures, relationships, and responsibilities for all 4 roles in the **Kingston Premier Football League (KPFL) Management System**.

---

## 1. High-Level System Architecture & Class Hierarchy

```mermaid
classDiagram
    direction TB

    %% ==========================================
    %% PERSON HIERARCHY (PERSON 2 SUPERCLASS)
    %% ==========================================
    class Person {
        <<abstract>>
        #string id
        #string name
        #int age
        #string gender
        +getId() string
        +getName() string
        +getAge() int
        +getGender() string
        +setName(string) void
        +setAge(int) void
        +setGender(string) void
        +displayInfo()* void
    }

    class Player {
        -string position
        -int jerseyNumber
        -int goals
        -string playingStatus
        -string teamId
        +getPosition() string
        +getJerseyNumber() int
        +getGoals() int
        +getPlayingStatus() string
        +addGoals(int) void
        +displayInfo() void
    }

    class Coach {
        -string qualification
        -string coachingRole
        -int yearsExperience
        -string teamId
        +getQualification() string
        +getCoachingRole() string
        +getYearsExperience() int
        +displayInfo() void
    }

    class MatchOfficial {
        -string role
        -string rank
        -bool available
        -vector~string~ bookedSlots
        +getRole() string
        +getRank() string
        +isAvailable() bool
        +isFree(date, time) bool
        +assignSlot(date, time) void
        +displayInfo() void
    }

    class LeagueCommissioner {
        +approveTeam(FootballTeam&) void
        +suspendTeam(FootballTeam&, bool) void
        +inspectMatch(Fixture&) bool
        +displayInfo() void
    }

    class Spectator {
        -string email
        -string phone
        -string preferredTeam
        -int ticketsBought
        +getEmail() string
        +getPhone() string
        +getPreferredTeam() string
        +incrementTickets(int) void
        +displayInfo() void
    }

    Person <|-- Player : Inheritance
    Person <|-- Coach : Inheritance
    Person <|-- MatchOfficial : Inheritance
    Person <|-- LeagueCommissioner : Inheritance
    Person <|-- Spectator : Inheritance

    %% ==========================================
    %% PRODUCT HIERARCHY (PERSON 4 SUPERCLASS)
    %% ==========================================
    class Product {
        <<abstract>>
        #string productId
        #string name
        #double unitPrice
        #int stockQuantity
        #string categoryType
        +getProductId() string
        +getName() string
        +getUnitPrice() double
        +getStockQuantity() int
        +getCategoryType() string
        +addStock(int) void
        +reduceStock(int) void
        +displayProduct()* void
    }

    class Merchandise {
        -string size
        -string teamRegNo
        -string material
        +getSize() string
        +getTeamRegNo() string
        +getMaterial() string
        +displayProduct() void
    }

    class FoodItem {
        -bool isBeverage
        -bool isPerishable
        -string expiryDate
        +getIsBeverage() bool
        +getIsPerishable() bool
        +getExpiryDate() string
        +displayProduct() void
    }

    Product <|-- Merchandise : Inheritance
    Product <|-- FoodItem : Inheritance

    %% ==========================================
    %% CORE LEAGUE & MATCH OPERATIONS
    %% ==========================================
    class FootballLeague {
        -string leagueName
        -LeagueCommissioner* commissioner
        -vector~FootballTeam*~ teams
        -vector~Stadium*~ stadiums
        -vector~MatchOfficial*~ officials
        -vector~Spectator*~ spectators
        -vector~unique_ptr~Fixture~~ fixtures
        +setCommissioner(LeagueCommissioner*) void
        +addTeam(FootballTeam*) void
        +addStadium(Stadium*) void
        +addOfficial(MatchOfficial*) void
        +addSpectator(Spectator*) void
        +scheduleFixture(home, away, stadium, date, time) Fixture&
        +assignOfficial(fixtureId, officialId) void
        +recordResult(fixtureId, homeScore, awayScore) void
        +getStandings() vector~FootballTeam*~
        +displayStandings() void
        +displayFixtures() void
    }

    class FootballTeam {
        -string regNumber
        -string name
        -string homeCity
        -bool registered
        -bool suspended
        -int played
        -int wins
        -int draws
        -int losses
        -int goalsFor
        -int goalsAgainst
        -int points
        -vector~Player~ squad
        -vector~Coach~ coachingStaff
        +recordMatch(gf, ga, pts) void
        +addPlayer(Player) void
        +removePlayer(string) bool
        +addCoach(Coach) void
        +removeCoach(string) bool
        +getGoalDifference() int
        +displayTeamInfo() void
    }

    class Stadium {
        -string stadiumId
        -string name
        -string location
        -int seatingCapacity
        -string fieldCondition
        -vector~string~ assignedTeamIds
        +assignTeam(string) void
        +setSeatingCapacity(int) void
        +setFieldCondition(string) void
        +displayInfo() void
    }

    class Fixture {
        -string fixtureId
        -string date
        -string time
        -FootballTeam* home
        -FootballTeam* away
        -Stadium* stadium
        -vector~MatchOfficial*~ officials
        -FixtureStatus status
        -unique_ptr~MatchResult~ result
        -vector~Ticket~ tickets
        +addOfficial(MatchOfficial*) void
        +generateTickets(bleacher, grandstand, vip) void
        +sellTicket(TicketCategory, Spectator&) Ticket*
        +setResult(MatchResult) void
        +complete() void
        +display() void
    }

    class MatchResult {
        -int homeScore
        -int awayScore
        +getHomeScore() int
        +getAwayScore() int
        +getHomePoints() int
        +getAwayPoints() int
    }

    class Ticket {
        -string ticketId
        -string fixtureId
        -string stadiumId
        -string seatNumber
        -TicketCategory category
        -double price
        -string spectatorId
        -bool isSold
        +sellTo(Spectator&) void
        +displayInfo() void
    }

    class StadiumShop {
        -string shopId
        -string stadiumId
        -vector~unique_ptr~Product~~ inventory
        -vector~Receipt~ transactionHistory
        +addProduct(unique_ptr~Product~) void
        +processSale(customer, cart, payment) Receipt
        +displayInventory() void
        +printReceipt(Receipt) void
    }

    %% Relationships
    FootballLeague o-- LeagueCommissioner : Aggregation
    FootballLeague *-- FootballTeam : Manages
    FootballLeague *-- Stadium : Manages
    FootballLeague *-- MatchOfficial : Manages
    FootballLeague *-- Fixture : Composition (1..*)
    FootballTeam *-- Player : Composition (1..*)
    FootballTeam *-- Coach : Composition (1..*)
    Fixture --> FootballTeam : References Home & Away
    Fixture --> Stadium : References
    Fixture o-- MatchOfficial : Aggregation (0..*)
    Fixture *-- MatchResult : Composition (0..1)
    Fixture *-- Ticket : Composition (1..*)
    Ticket ..> Spectator : Sold To
    StadiumShop *-- Product : Composition (1..*)
```

---

## 2. Shared OOP Principles & Implementation Matrix

| OOP Pillar | Design Implementation in Project | Example in Code |
| :--- | :--- | :--- |
| **Encapsulation** | Private/protected attributes with public accessors, mutators, and business invariant checks. | `Person::age` protected; `FootballTeam::points` mutated only via `recordMatch()`. |
| **Inheritance** | Single inheritance hierarchies for people and shop goods. | `Person` $\rightarrow$ `Player`, `Coach`, `MatchOfficial`, `LeagueCommissioner`, `Spectator`.<br>`Product` $\rightarrow$ `Merchandise`, `FoodItem`. |
| **Polymorphism** | Overriding pure virtual methods called via superclass references or pointers. | `virtual void displayInfo() const = 0;`<br>`virtual void displayProduct() const = 0;` |
| **Abstraction** | Abstract base classes that define common contracts without exposing internal details. | Abstract classes `Person` and `Product`. |
| **Exception Handling** | Custom hierarchy derived from `JFLException` and `std::runtime_error`. | `TeamNotEligibleException`, `SchedulingConflictException`, `CapacityExceededException`, `InsufficientPaymentException`. |
| **File Persistence** | Flat-file serialization and reconstruction. | `DataPersistence::saveTeams()`, `DataPersistence::loadTeams()`, saving shop receipts and inventory. |

---

## 3. Four-Person Module Breakdown & Ownership

```mermaid
flowchart LR
    subgraph Person1["PERSON 1: LEAGUE OPERATIONS"]
        FL[FootballLeague]
        LC[LeagueCommissioner]
        FX[Fixture]
        MR[MatchResult]
        MO[MatchOfficial]
    end

    subgraph Person2["PERSON 2: TEAM & PERSONNEL"]
        P[Person (Abstract)]
        PL[Player]
        CO[Coach]
        FT[FootballTeam]
    end

    subgraph Person3["PERSON 3: STADIUM & TICKETING"]
        ST[Stadium]
        TK[Ticket]
        SP[Spectator]
    end

    subgraph Person4["PERSON 4: SHOP, FINANCE & DATA"]
        PR[Product (Abstract)]
        ME[Merchandise]
        FI[FoodItem]
        SS[StadiumShop]
        DP[DataPersistence]
    end

    Person2 -.->|Person Base Class| Person1
    Person2 -.->|Person Base Class| Person3
    Person1 <-->|Fixtures & Stadiums| Person3
    Person3 <-->|Spectators & Sales| Person4
    Person1 <-->|Teams in League| Person2
```

1. **Person 1 (League Operations)**:
   - Owns `FootballLeague`, `LeagueCommissioner`, `Fixture`, `MatchResult`, `MatchOfficial`.
   - Controls scheduling, standings computation ($Win=3, Draw=1, Loss=0$), official conflict prevention.
2. **Person 2 (Team & Personnel Management)**:
   - Owns `Person` (Abstract base), `Player`, `Coach`, `FootballTeam`.
   - Enforces age criteria (Players $\ge 16$, Coaches $\ge 21$), jersey uniqueness (1–99), and roster tracking.
3. **Person 3 (Stadium & Ticket Management)**:
   - Owns `Stadium`, `Ticket`, `Spectator`.
   - Manages seating capacities, categories (`BLEACHERS`, `GRANDSTAND`, `VIP`), and prevents overselling past capacity.
4. **Person 4 (Stadium Shop, Finance & Data)**:
   - Owns `Product` (Abstract base), `Merchandise`, `FoodItem`, `StadiumShop`, `DataPersistence`.
   - Handles transactions, **15% GCT (General Consumption Tax)**, item stock, receipt generation, and flat file saving/reloading.
