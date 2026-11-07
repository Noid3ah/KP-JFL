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
    Person(int id, int age, string name, char gender) : id(id), name(name), age(age), gender(gender) {}


    int getID(){return id;}
    string getName(){return name;}
    int getAge(){return age;}
    char getGender(){return gender;}

    void setID(int id) {this->id = id;}
    void setName(string n){name = n;}
    void setAge(int a){age = a;}
    void setGender(char g){gender = g;}

    virtual void displayInfo() const = 0;
};

#endif