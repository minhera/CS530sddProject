#include "littab.h"
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

// Constructor
LiteralTable::LiteralTable() {}

// Check if literal already exists
bool LiteralTable::exists(const string& lit) const {
    for (const auto& l : literals) {
        if (l.name == lit) return true;
    }
    return false;
}

// Convert literal to hex value
static string literal_to_value(const string& lit) {
    if (lit.size() >= 4 && lit[0] == '=' && lit[1] == 'C') {
        string chars = lit.substr(3, lit.size() - 4);
        stringstream ss;

        for (char c : chars) {
            ss << uppercase << hex << setw(2) << setfill('0')
               << (int)(unsigned char)c;
        }

        return ss.str();
    }

    if (lit.size() >= 4 && lit[0] == '=' && lit[1] == 'X') {
        return lit.substr(3, lit.size() - 4);
    }

    return "";
}

// Compute literal length
static int literal_length(const string& lit) {
    if (lit.size() >= 4 && lit[0] == '=' && lit[1] == 'C') {
        return lit.size() - 4;
    }

    if (lit.size() >= 4 && lit[0] == '=' && lit[1] == 'X') {
        return (lit.size() - 4) / 2;
    }

    return 0;
}

// Add literal if not already present
void LiteralTable::add_literal(const string& lit) {
    if (exists(lit)) return;

    string value = literal_to_value(lit);
    int len = literal_length(lit);

    literals.push_back(Literal(lit, value, len));
}

// Assign addresses to unassigned literals
void LiteralTable::assign_literals(int& locctr, vector<pair<int, Literal>>& outputLines) {
    for (auto& lit : literals) {
        if (!lit.assigned) {
            lit.address = locctr;
            lit.assigned = true;

            // Save for listing output (address + literal)
            outputLines.push_back({locctr, lit});

            locctr += lit.length;
        }
    }
}

// Get literal address
int LiteralTable::get_address(const string& lit) const {
    for (const auto& l : literals) {
        if (l.name == lit) return l.address;
    }
    return 0;
}

// Get all literals
const vector<Literal>& LiteralTable::get_all() const {
    return literals;
}
