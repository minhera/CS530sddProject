#include "symtab.h"

bool SymbolTable::insert_symbol(const string &symbol, int locctr, bool CS = false){
    // Return false if the passed symbol is *
    if (symbol == "*"){
        return false;
    }

    // Check if symbol exists in the table
    if (symTab.count(symbol)){
        return false;
    }

    // If exist, update/insert the values to symbol storing map just in case
    symTab[symbol] = SymtabInfo(locctr, CS);
    symOrder.push_back(symbol);

    return true;
}

bool SymbolTable::lookup_symbol(const string &name, SymtabInfo& symtab){
    auto it = symTab.find(name);

    // If symbol is not found, return false
    if (it == symTab.end()){
        return false;
    }

    // If symbol is found, copy the Symbol Table Information to passed argument
    symtab = it->second;

    return true;
}

void SymbolTable::set_length(const string &name, int length){
    // Update Control Section's length for printing if exists
    auto it = symTab.find(name);

    if (it != symTab.end()){
        it->second.length = length;
    }
}


void SymbolTable::write_symtab(const string &filename, const string &CSName){

}