/**
 * Student 1 Name: Brian Madott
 * Student 1 Class account username: cssc2537
 * Student 2 Name: Minh Tran
 * Student 2 Class account username: cssc2552
 * Class: CS 530, Spring 2026
 * Assignment: Assignment #2 - Final SDD and Full Package
 * File Name: limited_assembler.h
 */

#ifndef LIMITED_ASSEMBLER_H
#define LIMITED_ASSEMBLER_H

#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include "optab.h"
#include "symtab.h"
#include "littab.h"

using namespace std;

// Stores one parsed line from the source program
struct SourceLine {
    int lineNumber;
    int address; // Current locctr value
    string label; 
    string opcode; // Mnemonic
    string operand; 
    string comment;
    string objectCode;
    bool isComment;

    SourceLine()
        : lineNumber(0), address(0), isComment(false) {}
};

class LimitedAssembler {
private:
    unordered_map<string, OptabInfo> optab;
    SymbolTable symtab;
    LiteralTable littab;
    vector<SourceLine> lines; // Collect ordered list of source code line by line

    string sourceFilename; 
    string programName;
    int startAddress;
    int programLength;
    int locctr;
    int baseRegister;
    bool baseEnabled;

    // Get the file path 
    string stFilePath;
    string lFilePath;

    // Helpers
    
    // Check if opcode is assembler directives
    bool is_directive(const string& opcode) const;
    int get_byte_length(const string& operand) const;

    // Converts integer to Hex || converts Hex to integer
    string int_to_hex(int value, int width) const;
    int hex_to_int(const string& hexStr) const;

    /**
     * @brief       Remove leading and trailing spaces, returns the substring
     */
    string trim(const string& s) const;
    void parse_line(const string& raw, SourceLine& line, int lineNumber);

    // Output helpers
    void print_listing_line(ofstream& out, const SourceLine& ln) const;
    void write_listing_file() const;
    void write_symtab_file() const;

public:
    LimitedAssembler(string &filename);

    // Main driver
    void assemble(const string& filename);

    // Both passes live in limited_assembler.cpp
    void pass1();
    void pass2();
};

#endif
