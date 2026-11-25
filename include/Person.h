#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "exceptions.h"

// Base class for everyone in the system — players, coaches, officials, spectators, etc.
// Can't be instantiated on its own since displayInfo() is pure virtual.
// All subclasses share the same id/name/age/gender fields defined here.
class Person {
protected:
    std::string id;
    std::string name;
    int age;
    std::string gender;

public:
    Person(const std::string& id, const std::string& name, int age, const std::string& gender)
        : id(id), name(name), age(age), gender(gender) {
        if (id.empty() || name.empty()) {
            throw std::invalid_argument("Person ID and name cannot be empty.");
        }
        if (age <= 0) {
            throw InvalidAgeException("Person age must be positive: " + std::to_string(age));
        }
    }

    virtual ~Person() = default;

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    int getAge() const { return age; }
    const std::string& getGender() const { return gender; }

    void setName(const std::string& newName) {
        if (newName.empty()) throw std::invalid_argument("Name cannot be empty.");
        name = newName;
    }

    void setAge(int newAge) {
        if (newAge <= 0) throw InvalidAgeException("Age must be positive: " + std::to_string(newAge));
        age = newAge;
    }

    void setGender(const std::string& newGender) { gender = newGender; }

    // each subclass decides how to display itself
    virtual void displayInfo() const = 0;
};
