#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "exceptions.h"

/**
 * =========================================================================
 * CLASS: Person (Person 2 - Abstract Superclass)
 * =========================================================================
 * PURPOSE:
 *  Base class for all human entities in the Jamaican Football League:
 *  Player, Coach, MatchOfficial, LeagueCommissioner, and Spectator.
 *
 * OOP CONCEPTS DEMONSTRATED:
 *  - Abstraction: Cannot be directly instantiated because displayInfo() is pure virtual.
 *  - Encapsulation: Protected fields (id, name, age, gender) with validated accessors/mutators.
 * =========================================================================
 */
class Person {
protected:
    std::string id;     ///< Unique identity shared across person subclasses.
    std::string name;   ///< Display name of the person.
    int age;            ///< Age in years; subclasses may impose higher minimums.
    std::string gender; ///< Gender label stored as supplied.

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

    // Getters
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    int getAge() const { return age; }
    const std::string& getGender() const { return gender; }

    // Setters
    void setName(const std::string& newName) {
        if (newName.empty()) throw std::invalid_argument("Name cannot be empty.");
        name = newName;
    }

    void setAge(int newAge) {
        if (newAge <= 0) throw InvalidAgeException("Age must be positive: " + std::to_string(newAge));
        age = newAge;
    }

    void setGender(const std::string& newGender) { gender = newGender; }

    // Pure virtual method: Polymorphic requirement
    virtual void displayInfo() const = 0;
};
