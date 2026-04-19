#include "optab.h"

unordered_map<string, OptabInfo> construct_optab(){
    unordered_map<string, OptabInfo> optab;

    // Construct opcode table taken from Appendix A in Textbook
    optab["ADD"] = {0x18, F3_4};
    optab["ADDF"] = {0x58, F3_4};
    optab["ADDR"] = {0x90, F2};
    optab["AND"] = {0x40, F3_4};
    optab["CLEAR"] = {0xB4, F2};
    optab["COMP"] = {0x28, F3_4};
    optab["COMPF"] = {0x88, F3_4};
    optab["COMPR"] = {0xA0, F2};
    optab["DIV"] = {0x24, F3_4};
    optab["DIVF"] = {0x64, F3_4};
    optab["DIVR"] = {0x9C, F2};
    optab["FIX"] = {0xC4, F1};
    optab["FLOAT"] = {0xC0, F1};
    optab["HIO"] = {0xF4, F1};
    optab["J"] = {0x3C, F3_4};
    optab["JEQ"] = {0x30, F3_4};
    optab["JGT"] = {0x34, F3_4};
    optab["JLT"] = {0x38, F3_4};
    optab["JSUB"] = {0x48, F3_4};
    optab["LDA"] = {0x00, F3_4};
    optab["LDB"] = {0x68, F3_4};
    optab["LDCH"] = {0x50, F3_4};
    optab["LDF"] = {0x70, F3_4};
    optab["LDL"] = {0x08, F3_4};
    optab["LDS"] = {0x6C, F3_4};
    optab["LDT"] = {0x74, F3_4};
    optab["LDX"] = {0x04, F3_4};
    optab["LPS"] = {0xD0, F3_4};
    optab["MUL"] = {0x20, F3_4};
    optab["MULF"] = {0x60, F3_4};
    optab["MULR"] = {0x98, F2};
    optab["NORM"] = {0xC8, F1};
    optab["OR"] = {0x44, F3_4};
    optab["RD"] = {0xD8, F3_4};
    optab["RMO"] = {0xAC, F2};
    optab["RSUB"] = {0x4C, F3_4};
    optab["SHIFTL"] = {0xA4, F2};
    optab["SHIFTR"] = {0xA8, F2};
    optab["SIO"] = {0xF0, F1};
    optab["SSK"] = {0xEC, F3_4};
    optab["STA"] = {0x0C, F3_4};
    optab["STB"] = {0x78, F3_4};
    optab["STCH"] = {0x54, F3_4};
    optab["STF"] = {0x80, F3_4};
    optab["STI"] = {0xD4, F3_4};
    optab["STL"] = {0x14, F3_4};
    optab["STS"] = {0x7C, F3_4};
    optab["STSW"] = {0xE8, F3_4};
    optab["STT"] = {0x84, F3_4};
    optab["STX"] = {0x10, F3_4};
    optab["SUB"] = {0x1C, F3_4};
    optab["SUBF"] = {0x5C, F3_4};
    optab["SUBR"] = {0x94, F2};
    optab["SVC"] = {0xB0, F2};
    optab["TD"] = {0xE0, F3_4};
    optab["TIO"] = {0xF8, F1};
    optab["TIX"] = {0x2C, F3_4};
    optab["TIXR"] = {0xB8, F2};
    optab["WD"] = {0xDC, F3_4};

    return optab;
}