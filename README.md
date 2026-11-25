# KP-JFL — Kingston Premier Football League Management System

A C++17 console application for managing a Jamaican football league, including fixtures, teams, stadiums, ticketing, and a match-day shop.

---

## Requirements

| Platform | Compiler |
|----------|----------|
| Windows  | MinGW-w64 `g++` (via [MSYS2](https://www.msys2.org/) or [WinLibs](https://winlibs.com/)) |
| macOS    | `clang++` via Xcode Command Line Tools |
| Linux    | `g++` — install with `sudo apt install g++` |

No third-party libraries needed — standard C++17 only.

---

## How to Compile

Open a terminal inside the project folder, then run:

**Windows (PowerShell or Command Prompt)**
```powershell
g++ -std=c++17 -Wall main.cpp -o main.exe
```

**macOS / Linux**
```bash
g++ -std=c++17 -Wall main.cpp -o main
```

---

## How to Run

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

Runs all 24 built-in business rule tests automatically:

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

> Login accounts are saved locally to `kpfl_users.txt` and persist between sessions.

---

## Data Files

The program automatically creates and updates these files in the project folder:

| File | Contents |
|------|----------|
| `kpfl_teams.txt` | Team registrations and standings |
| `kpfl_players.txt` | Team squads |
| `kpfl_coaches.txt` | Coaching staff |
| `kpfl_fixtures.txt` | Fixtures, officials and results |
| `kpfl_spectators.txt` | Registered spectators |
| `kpfl_tickets.txt` | Match tickets and sales |
| `kpfl_stadiums.txt` | Stadium info and capacity |
| `kpfl_shop_inventory.txt` | Shop product stock levels |
| `kpfl_shop_receipts.txt` | Sales receipts and transactions |
| `kpfl_users.txt` | Login accounts *(local only — not committed to git)* |
