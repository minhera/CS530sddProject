/**
 * Student 1 Name: Brian Madott
 * Student 1 Class account username: cssc2537
 * Student 2 Name: Minh Tran
 * Student 2 Class account username: cssc2552
 * Class: CS 530, Spring 2026
 * Assignment: Assignment #2 - Final SDD and Full Package
 * File Name: littab.h
 */

#ifndef LITTAB_H
#define LITTAB_H

#include <string>
#include <vector>

using namespace std;

// Represents one literal entry
struct Literal {
    string name;     // e.g. =C'EOF'
    string value;    // e.g. 454F46
    int length;      // number of bytes
    int address;     // assigned during LTORG or END
    bool assigned;   // whether address is assigned

    Literal(string n, string v, int len)
        : name(n), value(v), length(len), address(0), assigned(false) {}
};

class LiteralTable {
private:
    vector<Literal> literals;

public:
    LiteralTable();

    // Add literal if not already present
    void add_literal(const string& lit);

    // Assign addresses starting at LOCCTR
    void assign_literals(int& locctr, vector<pair<int, Literal>>& outputLines);

    // Check if literal exists
    bool exists(const string& lit) const;

    // Get literal address
    int get_address(const string& lit) const;

    // Get all literals (for printing)
    const vector<Literal>& get_all() const;
};

#endif
