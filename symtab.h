/**
 * Student 1 Name: Brian Madott
 * Student 1 Class account username: cssc2537
 * Student 2 Name: Minh Tran
 * Student 2 Class account username: cssc2552
 * Class: CS 530, Spring 2026
 * Assignment: Assignment #2 - Final SDD and Full Package
 * File Name: symtab.h
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct SymtabInfo {
    int address;
    bool isAbsolute;

    SymtabInfo(int addr = 0, bool abs = false)
        : address(addr), isAbsolute(abs) {}
};

class SymbolTable {
private:
    unordered_map<string, SymtabInfo> symTab;
    vector<string> symOrder;
    int programLength;

public:
    SymbolTable();

    bool insert_symbol(const string& symbol, int locctr, bool CS = false);
    bool lookup_symbol(const string& symbol, SymtabInfo& info) const;
    void write_symtab(const string& filename, const string& CSName) const;
    void set_length(int len);
};

#endif
