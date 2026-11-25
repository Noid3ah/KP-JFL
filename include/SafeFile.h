#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include "exceptions.h"

using namespace std;

// writes to a temp file first so a failed write can't damage the saved data
inline void writeFileSafely(const string& content, const string& filename) {
    const string temp = filename + ".tmp";
    ofstream file(temp);
    if (!file.is_open()) throw PersistenceException("Cannot open file for writing: " + temp);
    file << content;
    file.close();
    if (!file) throw PersistenceException("Failed writing: " + temp);
    filesystem::rename(temp, filename);
}
