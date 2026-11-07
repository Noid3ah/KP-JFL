#ifndef TEAMS_PERSONNEL_H
#define TEAMS_PERSONNEL_H

#include <iostream>
#include <string>
#include <vector>
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

    int getID(){return id;}
    string getName(){return name;}
    int getAge(){return age;}
    char getGender(){return gender;}

    void setID(int id) {this->id = id;}
    void setName(string n){name = n;}
    void setAge(int a){age = a;}
    void setGender(char g){gender = g;}

    virtual void displayInfo() = 0;
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

    string getPosition() {return position;}
    int getJerseyNumber() {return jerseyNumber;}
    int getGoals() {return goals;}
    bool getPlayingStatus() {return isPlaying;}

    void setPosition(string p) {position = p;}
    void setJerseyNumber(int j) {jerseyNumber = j;}
    void setGoals(int g) {goals = g;}
    void setPlayingStatus(bool ps) {isPlaying = ps;}
    void scoreGoal() {goals++;}

    void displayInfo() override {
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

    string getQualifications() {return qualifications;}
    string getRole() {return role;}
    string getTeamAssigned() {return teamAssigned;}

    void setQualifications(string q) {qualifications = q;}
    void setRole(string r) {role = r;}
    void setTeamAssigned(string t) {teamAssigned = t;}

    void displayInfo() override {
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

    string getTeamName() {return teamName;}
    string getParish() {return parish;}
    int getRegistrationNumber() {return registrationNumber;}
    string getLeagueStatus() {return leagueStatus;}
    int getSqadSize() {return players.size();}

    vector<Player> getPlayers() {return players;}
    vector<Coach> getCoaches() {return coaches;}

    void setTeamName(string tn) {teamName = tn;}
    void setParish(string p) {parish = p;}
    void setRegistrationNumber(int rn) {registrationNumber = rn;}
    void setLeagueStatus(string ls) {leagueStatus = ls;}

    void addPlayer(const Player& player) {players.push_back(player);}
    void updatePlayer(Player& player) {
        for (auto& p : players) {
            if (p.getID() == player.getID()) {
                p = player;
                return;
            }
        }
    }
    // void removePlayer(int playerId) {
    //     players.erase(remove_if(players.begin(), players.end(), [playerId](const Player& p) {
    //         return p.getID() == playerId;
    //     }), players.end());
    // }
};

#endif