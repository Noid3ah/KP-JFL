#ifndef TEAMS_PERSONNEL_H
#define TEAMS_PERSONNEL_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class Person {
private:
    int id;
    string name;
    int age;
    char gender;

public:
    Person() {}
    Person(int id, string name, int age, char gender) : id(id), name(name), age(age), gender(gender) {}

    int getID() const {return id;}
    string getName() const {return name;}
    int getAge() const {return age;}
    char getGender() const {return gender;}

    void setID(int id) {this->id = id;}
    void setName(string n){name = n;}
    void setAge(int a){age = a;}
    void setGender(char g){gender = g;}

    virtual void displayInfo() const = 0;
};

class Player : public Person {
private:
    string position;
    int jerseyNumber;
    int goals;
    bool isPlaying;

public:
    Player(int id, string name, int age, char gender, string position, int jerseyNumber, int goals, bool isPlaying) 
        : Person(id, name, age, gender), position(position), jerseyNumber(jerseyNumber), goals(goals), isPlaying(isPlaying) {}

    string getPosition() const {return position;}
    int getJerseyNumber() const {return jerseyNumber;}
    int getGoals() const {return goals;}
    bool getPlayingStatus() const {return isPlaying;}

    void setPosition(string p) {position = p;}
    void setJerseyNumber(int j) {jerseyNumber = j;}
    void setGoals(int g) {goals = g;}
    void setPlayingStatus(bool ps) {isPlaying = ps;}
    void scoreGoal() {goals++;}

    void displayInfo() const override {
        cout << "Player ID: " << getID() << endl;
        cout << "Name: " << getName() << endl;
        cout << "Age: " << getAge() << endl;
        cout << "Gender: " << getGender() << endl;
        cout << "Position: " << position << endl;
        cout << "Jersey Number: " << jerseyNumber << endl;
        cout << "Goals Scored: " << goals << endl;
        cout << "Playing Status: " << (isPlaying ? "Active" : "Inactive") << endl;
    }
};

class Coach : public Person {
private:
    string qualifications;
    string role;
    string teamAssigned;

public:
    Coach(int id, string name, int age, char gender, string qualifications, string role, string teamAssigned) 
        : Person(id, name, age, gender), qualifications(qualifications), role(role), teamAssigned(teamAssigned) {}

    string getQualifications() const {return qualifications;}
    string getRole() const {return role;}
    string getTeamAssigned() const {return teamAssigned;}

    void setQualifications(string q) {qualifications = q;}
    void setRole(string r) {role = r;}
    void setTeamAssigned(string t) {teamAssigned = t;}

    void displayInfo() const override {
        cout << "Coach ID: " << getID() << endl;
        cout << "Name: " << getName() << endl;
        cout << "Age: " << getAge() << endl;
        cout << "Gender: " << getGender() << endl;
        cout << "Qualifications: " << qualifications << endl;
        cout << "Role: " << role << endl;
        cout << "Team Assigned: " << teamAssigned << endl;
    }
};

class Team {
private:
    string teamName;
    string parish;
    int registrationNumber;
    string leagueStatus;
    vector<Player> players;
    vector<Coach> coaches;
public:
    Team(string teamName, string parish, int registrationNumber, string leagueStatus) 
        : teamName(teamName), parish(parish), registrationNumber(registrationNumber), leagueStatus(leagueStatus) {}

    string getTeamName() const {return teamName;}
    string getParish() const {return parish;}
    int getRegistrationNumber() const {return registrationNumber;}
    string getLeagueStatus() const {return leagueStatus;}
    int getSqadSize() const {return players.size();}

    vector<Player> getPlayers() const {return players;}
    vector<Coach> getCoaches() const {return coaches;}

    void setTeamName(string tn) {teamName = tn;}
    void setParish(string p) {parish = p;}
    void setRegistrationNumber(int rn) {registrationNumber = rn;}
    void setLeagueStatus(string ls) {leagueStatus = ls;}

    void addPlayer(const Player& player) {players.push_back(player);}
    void updatePlayer(const Player& player) {
        for (auto& p : players) {
            if (p.getID() == player.getID()) {
                p = player;
                return;
            }
        }
    }
    void removePlayer(int playerId) {
        players.erase(remove_if(players.begin(), players.end(), [playerId](const Player& p) {
            return p.getID() == playerId;
        }), players.end());
    }

    Player* searchPlayer(int playerId) {
        for (auto& p : players) {
            if (p.getID() == playerId) {
                return &p;
            }
        }
        return nullptr;
    }

    void addCoach(const Coach& coach) {coaches.push_back(coach);}
    void updateCoach(const Coach& coach) {
        for (auto& c : coaches) {
            if (c.getID() == coach.getID()) {
                c = coach;
                return;
            }
        }
    }
    void removeCoach(int coachId) {
        coaches.erase(remove_if(coaches.begin(), coaches.end(), [coachId](const Coach& c) {
            return c.getID() == coachId;
        }), coaches.end());
    }
};

#endif