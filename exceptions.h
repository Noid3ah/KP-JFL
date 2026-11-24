#pragma once
#include <stdexcept>
#include <string>

// Base class for every business-rule violation in the league module.
class LeagueException : public std::runtime_error {
public:
    explicit LeagueException(const std::string& msg) : std::runtime_error(msg) {}
};

// Team is unregistered or suspended.
class TeamNotEligibleException : public LeagueException {
public:
    explicit TeamNotEligibleException(const std::string& msg) : LeagueException(msg) {}
};

// Team (or stadium) already booked for the same date and time.
class SchedulingConflictException : public LeagueException {
public:
    explicit SchedulingConflictException(const std::string& msg) : LeagueException(msg) {}
};

// Official already assigned to another fixture at the same date and time.
class OfficialConflictException : public LeagueException {
public:
    explicit OfficialConflictException(const std::string& msg) : LeagueException(msg) {}
};

// Negative score.
class InvalidScoreException : public LeagueException {
public:
    explicit InvalidScoreException(const std::string& msg) : LeagueException(msg) {}
};

// Fixture completed without a valid result.
class ResultRequiredException : public LeagueException {
public:
    explicit ResultRequiredException(const std::string& msg) : LeagueException(msg) {}
};
