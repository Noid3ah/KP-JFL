#pragma once
#include <string>

using namespace std;

// Format checks for typed input. Each returns true when the text is acceptable.

// letters, spaces, hyphens, apostrophes and full stops, with at least one letter
// bytes above 127 are allowed so accented names work
inline bool isValidPersonName(const string& name) {
    bool hasLetter = false;
    for (unsigned char ch : name) {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch >= 128) hasLetter = true;
        else if (ch != ' ' && ch != '-' && ch != '\'' && ch != '.') return false;
    }
    return hasLetter;
}

// something@domain.tld, with no spaces
inline bool isValidEmail(const string& email) {
    if (email.find_first_of(" \t|") != string::npos) return false;
    const size_t at = email.find('@');
    if (at == string::npos || at == 0 || email.find('@', at + 1) != string::npos) return false;
    const string domain = email.substr(at + 1);
    return domain.find('.') != string::npos && domain.front() != '.' && domain.back() != '.'
        && domain.find("..") == string::npos;
}

// 7 to 15 digits; spaces, hyphens, brackets and a leading + are allowed
inline bool isValidPhone(const string& phone) {
    int digits = 0;
    for (size_t i = 0; i < phone.size(); ++i) {
        const char ch = phone[i];
        if (ch >= '0' && ch <= '9') ++digits;
        else if (ch == '+' ? i != 0 : (ch != ' ' && ch != '-' && ch != '(' && ch != ')')) return false;
    }
    return digits >= 7 && digits <= 15;
}

// plain digits with at most two decimals, e.g. 8500 or 8500.50
inline bool isValidMoney(const string& text) {
    const size_t dot = text.find('.');
    if (text.empty() || text.size() > 16 || dot == 0 || text.find_first_not_of("0123456789.") != string::npos) return false;
    if (dot == string::npos) return true;
    const size_t decimals = text.size() - dot - 1;
    return text.find('.', dot + 1) == string::npos && decimals >= 1 && decimals <= 2;
}
