#pragma once
#include <string>
#include <stdexcept>

// TEMPORARY STUB - Person 2 owns Person. Keep these signatures when replacing it.
class Person {
private:
    std::string id, name, gender;
    int age;
public:
    Person(const std::string& id, const std::string& name, int age, const std::string& gender)
        : id(id), name(name), gender(gender), age(age) {
        if (id.empty() || name.empty()) throw std::invalid_argument("Person needs an id and a name");
        if (age <= 0) throw std::invalid_argument("Age must be positive");
    }
    virtual ~Person() = default;

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    int getAge() const { return age; }
    const std::string& getGender() const { return gender; }

    virtual void displayInfo() const = 0;
};
