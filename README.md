# KP-JFL — Kingston Premier Football League Management System

A C++17 console application for managing a Jamaican football league, including fixtures, teams, stadiums, ticketing, and a match-day shop.

---

## Requirements

| Platform | Compiler                                                                                 |
| -------- | ---------------------------------------------------------------------------------------- |
| Windows  | MinGW-w64 `g++` (via [MSYS2](https://www.msys2.org/) or [WinLibs](https://winlibs.com/)) |
| macOS    | `clang++` via Xcode Command Line Tools                                                   |
| Linux    | `g++` — install with `sudo apt install g++`                                              |

No third-party libraries needed — standard C++17 only.

---

## How to Compile

Open a terminal inside the project folder, then run:

**Windows (PowerShell or Command Prompt)**

```powershell
g++ -std=c++17 -Wall src/main.cpp -o main.exe
```

**macOS / Linux**

```bash
g++ -std=c++17 -Wall src/main.cpp -o main
```

---

## How to Run

Run from the project folder. Data files are read from and written to whichever folder you run from.

**Windows**

```powershell
.\main.exe
```

**macOS / Linux**

```bash
./main
```

---

## Automated Test Mode

Runs all 36 built-in business rule tests automatically:

**Windows**

```powershell
.\main.exe --test
```

**macOS / Linux**

```bash
./main --test
```

---

## Login

When the program starts you will be prompted to log in.  
Default credentials are listed on the login screen.

> Login accounts are saved to `kpfl_users.txt` and persist between sessions.

---

## Data Files

The program saves to these files as you make changes, and reads them back at startup. Some appear only after the first related change. They are not committed to git; without them the program starts from built-in sample data.

| File                      | Contents                         |
| ------------------------- | -------------------------------- |
| `kpfl_teams.txt`          | Team registrations and standings |
| `kpfl_players.txt`        | Team squads                      |
| `kpfl_coaches.txt`        | Coaching staff                   |
| `kpfl_fixtures.txt`       | Fixtures, officials and results  |
| `kpfl_spectators.txt`     | Registered spectators            |
| `kpfl_tickets.txt`        | Match tickets and sales          |
| `kpfl_stadiums.txt`       | Stadium info and capacity        |
| `kpfl_shop_inventory.txt` | Shop product stock levels        |
| `kpfl_shop_receipts.txt`  | Sales receipts and transactions  |
| `kpfl_users.txt`          | Login accounts                   |

The files refer to each other, so keep or delete them as a set. To go back to the sample data, delete every `kpfl_*.txt` file except `kpfl_users.txt`.

If a file is damaged, the program stops at startup and names the file and line to fix.

---

## Contributors

| Name | Area                           |
| ---- | ------------------------------ |
| AF   | League Operations & Fixtures   |
| OP   | Team & Personnel Management    |
| KL   | Stadium Operations & Ticketing |
| KB   | Stadium Shop, Finance & Data   |
