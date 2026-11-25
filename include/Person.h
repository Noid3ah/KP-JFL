#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "exceptions.h"

using namespace std;

// Base class for everyone in the system — players, coaches, officials, spectators, etc.
// Can't be instantiated on its own since displayInfo() is pure virtual.
// All subclasses share the same id/name/age/gender fields defined here.
class Person {
protected:
    string id;
    string name;
    int age;
    string gender;

public:
    Person(const string& id, const string& name, int age, const string& gender)
        : id(id), name(name), age(age), gender(gender) {
        if (id.empty() || name.empty()) {
            throw invalid_argument("Person ID and name cannot be empty.");
        }
        if (age <= 0) {
            throw InvalidAgeException("Person age must be positive: " + to_string(age));
        }
    }

    virtual ~Person() = default;

    const string& getId() const { return id; }
    const string& getName() const { return name; }
    int getAge() const { return age; }
    const string& getGender() const { return gender; }

    void setName(const string& newName) {
        if (newName.empty()) throw invalid_argument("Name cannot be empty.");
        name = newName;
    }

    void setAge(int newAge) {
        if (newAge <= 0) throw InvalidAgeException("Age must be positive: " + to_string(newAge));
        age = newAge;
    }

    void setGender(const string& newGender) { gender = newGender; }

    // each subclass decides how to display itself
    virtual void displayInfo() const = 0;
};
