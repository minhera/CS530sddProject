#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <fstream> // Writing to file
#include <unordered_map>
#include <vector>
using namespace std;

struct SymtabInfo{
    bool isCSect; // Check if the symbol is the control section (prints different stuff)
    bool isAbsolute;
    int address; 
    int length; // For Control Section print statement

    // Constructors
    SymtabInfo() : address(0), isCSect(false), length(0), isAbsolute(false) {}
    SymtabInfo(int value, bool cs = false, bool absolute = false) : address(value), isCSect(cs), length(0), isAbsolute(absolute) {}
};

class SymbolTable{
private:
    unordered_map<string, SymtabInfo> symTab;
    vector<string> symOrder;
public:
    SymbolTable() {};

    // insert to table, look up symbol, write to file
    bool insert_symbol(const string &symbol, int locctr, bool CS = false);
    bool lookup_symbol(const string &name, SymtabInfo& symtab);
    void set_length(const string &name, int length); // for END statement
    void write_symtab(const string &filename, const string &CSName);

};

#endif