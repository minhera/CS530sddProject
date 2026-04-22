/**
 * Student 1 Name: Brian Madott
 * Student 1 Class account username: cssc2537
 * Student 2 Name: Minh Tran
 * Student 2 Class account username: cssc2552
 * Class: CS 530, Spring 2026
 * Assignment: Assignment #2 - Final SDD and Full Package
 * File Name: symtab.cpp
 */

#include "symtab.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

SymbolTable::SymbolTable() : programLength(0) {}

bool SymbolTable::insert_symbol(const string& symbol, int locctr, bool CS) {
    if (symTab.count(symbol)) {
        return false;
    }

    symTab[symbol] = SymtabInfo(locctr, CS);
    symOrder.push_back(symbol);
    return true;
}

bool SymbolTable::lookup_symbol(const string& symbol, SymtabInfo& info) const {
    auto it = symTab.find(symbol);
    if (it == symTab.end()) {
        return false;
    }

    info = it->second;
    return true;
}

void SymbolTable::set_length(int len) {
    programLength = len;
}

void SymbolTable::write_symtab(const string& filename, const string& CSName) const {
    ofstream out(filename);

    // Try opening the files
    if (!out.is_open()) {
        cerr << "Error: could not open symtab output file " << filename << endl;
        return;
    }

    out << "CSect   Symbol  Value   LENGTH  Flags:\n";
    out << "--------------------------------------\n";

    // Print control section line
    out << left << setw(8) << CSName
        << setw(8) << ""
        << right << uppercase << hex << setw(6) << setfill('0') << 0
        << "  "
        << setw(6) << setfill('0') << programLength
        << setfill(' ') << "\n";

    // Print symbols in insertion order, skipping the control section symbol itself
    for (const string& symbol : symOrder) {
        if (symbol == CSName) continue;

        auto it = symTab.find(symbol);
        if (it == symTab.end()) continue;

        out << left << setw(8) << ""
            << setw(8) << symbol
            << right << uppercase << hex << setw(6) << setfill('0') << it->second.address
            << setfill(' ') << "          "
            << "R"
            << "\n";
    }
}
