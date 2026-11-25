#pragma once
#include <cstdlib>
#include <string>

using namespace std;

// Checks and comparisons for match dates (YYYY-MM-DD) and times (HH:MM).

// matches that share a team, stadium or official must start at least this far apart
constexpr int MIN_MINUTES_APART = 120;

// checks that text matches a pattern where 'd' stands for any digit
inline bool matchesDigits(const string& text, const string& pattern) {
    if (text.size() != pattern.size()) return false;
    for (size_t i = 0; i < text.size(); ++i) {
        const bool digit = text[i] >= '0' && text[i] <= '9';
        if (pattern[i] == 'd' ? !digit : text[i] != pattern[i]) return false;
    }
    return true;
}

// a real calendar date as YYYY-MM-DD
inline bool isValidDate(const string& date) {
    if (!matchesDigits(date, "dddd-dd-dd")) return false;
    const int year = stoi(date.substr(0, 4));
    const int month = stoi(date.substr(5, 2));
    const int day = stoi(date.substr(8, 2));
    if (year < 1 || month < 1 || month > 12) return false;
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int days[] = {31, leap ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return day >= 1 && day <= days[month - 1];
}

// a 24-hour time as HH:MM
inline bool isValidTime(const string& time) {
    return matchesDigits(time, "dd:dd") && stoi(time.substr(0, 2)) < 24 && stoi(time.substr(3, 2)) < 60;
}

// minutes since a fixed starting point, so slots on different days can be compared
inline long long slotMinutes(const string& date, const string& time) {
    int year = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    // counting the year from March puts the leap day last
    if (month <= 2) { --year; month += 12; }
    const long long days = 365LL * year + year / 4 - year / 100 + year / 400 + (153 * (month - 3) + 2) / 5
                         + stoi(date.substr(8, 2));
    return days * 24 * 60 + stoi(time.substr(0, 2)) * 60 + stoi(time.substr(3, 2));
}

// true if two matches start too close together
inline bool slotsOverlap(const string& date1, const string& time1, const string& date2, const string& time2) {
    // older saved fixtures may hold text that isn't a real date, which can only be compared as text
    if (!isValidDate(date1) || !isValidTime(time1) || !isValidDate(date2) || !isValidTime(time2)) {
        return date1 == date2 && time1 == time2;
    }
    return llabs(slotMinutes(date1, time1) - slotMinutes(date2, time2)) < MIN_MINUTES_APART;
}
