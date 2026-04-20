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
    int address;
    string label;
    string opcode;
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
    vector<SourceLine> lines;

    string sourceFilename;
    string programName;
    int startAddress;
    int programLength;
    int locctr;
    int baseRegister;
    bool baseEnabled;

    // Helpers
    bool is_directive(const string& opcode) const;
    int get_byte_length(const string& operand) const;
    string int_to_hex(int value, int width) const;
    int hex_to_int(const string& hexStr) const;
    string trim(const string& s) const;
    void parse_line(const string& raw, SourceLine& line, int lineNumber);

    // Output helpers
    void write_listing_file() const;
    void write_symtab_file() const;

public:
    LimitedAssembler();

    // Main driver
    void assemble(const string& filename);

    // Both passes live in limited_assembler.cpp
    void pass1();
    void pass2();
};

#endif
