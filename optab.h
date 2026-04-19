#ifndef OPTAB_H
#define OPTAB_H

#include <unordered_map>
#include <string>
using namespace std;

enum FormatType{
    F1 = 1,
    F2 = 2,
    F3_4 = 3 // Will be 4 with the + sign in the source code
};

struct OptabInfo{
    int opcode;
    FormatType format;
};

unordered_map<string, OptabInfo> construct_optab();

#endif