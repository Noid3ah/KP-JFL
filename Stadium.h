#pragma once
#include <string>

// TEMPORARY STUB - Person 3 owns Stadium. The league module only needs these two getters.
class Stadium {
private:
    std::string stadiumId, name;
public:
    Stadium(const std::string& id, const std::string& name) : stadiumId(id), name(name) {}
    const std::string& getStadiumId() const { return stadiumId; }
    const std::string& getName() const { return name; }
};
