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

    string getPosition(){return position;}
    int getJerseyNumber(){return jerseyNumber;}
    int getGoals(){return goals;}
    bool getPlayingStatus(){return isPlaying;}

    void setPosition(string p){position = p;}
    void setJerseyNumber(int j){jerseyNumber = j;}
    void setGoals(int g){goals = g;}
    void setPlayingStatus(bool ps){isPlaying = ps;}
    void scoreGoal(){goals++;}

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

#endif