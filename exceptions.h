#pragma once
#include <stdexcept>
#include <string>

/// Base exception for all domain and validation errors in the application.
class JFLException : public std::runtime_error {
public:
    explicit JFLException(const std::string& msg) : std::runtime_error(msg) {}
};

/// League operations and scheduling failures.
class LeagueException : public JFLException {
public:
    explicit LeagueException(const std::string& msg) : JFLException(msg) {}
};

class TeamNotEligibleException : public LeagueException {
public:
    explicit TeamNotEligibleException(const std::string& msg) : LeagueException(msg) {}
};

class SchedulingConflictException : public LeagueException {
public:
    explicit SchedulingConflictException(const std::string& msg) : LeagueException(msg) {}
};

class OfficialConflictException : public LeagueException {
public:
    explicit OfficialConflictException(const std::string& msg) : LeagueException(msg) {}
};

class InvalidScoreException : public LeagueException {
public:
    explicit InvalidScoreException(const std::string& msg) : LeagueException(msg) {}
};

class ResultRequiredException : public LeagueException {
public:
    explicit ResultRequiredException(const std::string& msg) : LeagueException(msg) {}
};

/// Team, player, coach, and personnel management failures.
class PersonnelException : public JFLException {
public:
    explicit PersonnelException(const std::string& msg) : JFLException(msg) {}
};

class InvalidAgeException : public PersonnelException {
public:
    explicit InvalidAgeException(const std::string& msg) : PersonnelException(msg) {}
};

class DuplicatePersonException : public PersonnelException {
public:
    explicit DuplicatePersonException(const std::string& msg) : PersonnelException(msg) {}
};

/// Stadium and ticketing failures.
class StadiumTicketingException : public JFLException {
public:
    explicit StadiumTicketingException(const std::string& msg) : JFLException(msg) {}
};

class CapacityExceededException : public StadiumTicketingException {
public:
    explicit CapacityExceededException(const std::string& msg) : StadiumTicketingException(msg) {}
};

/// Stadium-shop, payment, stock, and persistence failures.
class ShopFinanceException : public JFLException {
public:
    explicit ShopFinanceException(const std::string& msg) : JFLException(msg) {}
};

class InsufficientPaymentException : public ShopFinanceException {
public:
    explicit InsufficientPaymentException(const std::string& msg) : ShopFinanceException(msg) {}
};

class OutOfStockException : public ShopFinanceException {
public:
    explicit OutOfStockException(const std::string& msg) : ShopFinanceException(msg) {}
};

class PersistenceException : public JFLException {
public:
    explicit PersistenceException(const std::string& msg) : JFLException(msg) {}
};
