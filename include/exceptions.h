#pragma once
#include <stdexcept>
#include <string>

using namespace std;

// root exception — everything in the system extends from this
class JFLException : public runtime_error {
public:
    explicit JFLException(const string& msg) : runtime_error(msg) {}
};

// anything that goes wrong with fixtures, scheduling, or results
class LeagueException : public JFLException {
public:
    explicit LeagueException(const string& msg) : JFLException(msg) {}
};

class TeamNotEligibleException : public LeagueException {
public:
    explicit TeamNotEligibleException(const string& msg) : LeagueException(msg) {}
};

class SchedulingConflictException : public LeagueException {
public:
    explicit SchedulingConflictException(const string& msg) : LeagueException(msg) {}
};

class OfficialConflictException : public LeagueException {
public:
    explicit OfficialConflictException(const string& msg) : LeagueException(msg) {}
};

class InvalidScoreException : public LeagueException {
public:
    explicit InvalidScoreException(const string& msg) : LeagueException(msg) {}
};

class ResultRequiredException : public LeagueException {
public:
    explicit ResultRequiredException(const string& msg) : LeagueException(msg) {}
};

// covers age violations, duplicate IDs, jersey conflicts, etc.
class PersonnelException : public JFLException {
public:
    explicit PersonnelException(const string& msg) : JFLException(msg) {}
};

class InvalidAgeException : public PersonnelException {
public:
    explicit InvalidAgeException(const string& msg) : PersonnelException(msg) {}
};

class DuplicatePersonException : public PersonnelException {
public:
    explicit DuplicatePersonException(const string& msg) : PersonnelException(msg) {}
};

// thrown when ticket or capacity rules are broken
class StadiumTicketingException : public JFLException {
public:
    explicit StadiumTicketingException(const string& msg) : JFLException(msg) {}
};

class CapacityExceededException : public StadiumTicketingException {
public:
    explicit CapacityExceededException(const string& msg) : StadiumTicketingException(msg) {}
};

// shop-related issues — bad payment, out of stock, file errors
class ShopFinanceException : public JFLException {
public:
    explicit ShopFinanceException(const string& msg) : JFLException(msg) {}
};

class InsufficientPaymentException : public ShopFinanceException {
public:
    explicit InsufficientPaymentException(const string& msg) : ShopFinanceException(msg) {}
};

class OutOfStockException : public ShopFinanceException {
public:
    explicit OutOfStockException(const string& msg) : ShopFinanceException(msg) {}
};

class PersistenceException : public JFLException {
public:
    explicit PersistenceException(const string& msg) : JFLException(msg) {}
};

// registering, logging in or changing a password was refused
class AuthException : public JFLException {
public:
    explicit AuthException(const string& msg) : JFLException(msg) {}
};
