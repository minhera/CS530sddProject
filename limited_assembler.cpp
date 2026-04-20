#include "limited_assembler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

// Constructor initializes default values and loads opcode table
LimitedAssembler::LimitedAssembler() {
    optab = construct_optab();   // Load opcode table from optab.cpp
    sourceFilename = "";
    programName = "";
    startAddress = 0;
    programLength = 0;
    locctr = 0;
    baseRegister = 0;
    baseEnabled = false;
}

// Removes whitespace from both ends of a string
string LimitedAssembler::trim(const string& s) const {
    size_t start = s.find_first_not_of(" \t\r\n");  // first non-space
    if (start == string::npos) return "";          // string is all whitespace

    size_t end = s.find_last_not_of(" \t\r\n");    // last non-space
    return s.substr(start, end - start + 1);       // trimmed substring
}

// Converts integer to uppercase hex string
string LimitedAssembler::int_to_hex(int value, int width) const {
    stringstream ss;
    ss << uppercase << hex << setw(width) << setfill('0')
       << (value & ((1 << (width * 4)) - 1));     // mask to correct bit size
    return ss.str();
}

// Converts hex string into integer
int LimitedAssembler::hex_to_int(const string& hexStr) const {
    int value = 0;
    stringstream ss;
    ss << hex << hexStr;  // load hex string
    ss >> value;          // convert to integer
    return value;
}

// Checks if opcode is an assembler directive (not machine instruction)
bool LimitedAssembler::is_directive(const string& opcode) const {
    return opcode == "START" || opcode == "END" ||
           opcode == "WORD"  || opcode == "RESW" ||
           opcode == "RESB"  || opcode == "BYTE" ||
           opcode == "BASE"  || opcode == "NOBASE" ||
           opcode == "LTORG";
}

// Calculates number of bytes required for BYTE directive
int LimitedAssembler::get_byte_length(const string& operand) const {
    if (operand.size() < 3) return 0;

    // Character constant: C'EOF' → 3 bytes
    if (operand[0] == 'C' && operand[1] == '\'' && operand.back() == '\'') {
        return operand.size() - 3;
    }

    // Hex constant: X'F1' → 1 byte
    if (operand[0] == 'X' && operand[1] == '\'' && operand.back() == '\'') {
        return (operand.size() - 3) / 2;
    }

    return 0;
}

// Parses one raw line from the source file into structured fields
void LimitedAssembler::parse_line(const string& raw, SourceLine& line, int lineNumber) {
    line.lineNumber = lineNumber;
    line.address = 0;
    line.label = "";
    line.opcode = "";
    line.operand = "";
    line.comment = "";
    line.objectCode = "";
    line.isComment = false;

    string cleaned = trim(raw);

    // Blank line
    if (cleaned.empty()) {
        line.isComment = true;
        return;
    }

    // Comment line
    if (cleaned[0] == '.') {
        line.isComment = true;
        line.comment = cleaned;
        return;
    }

    // Split into whitespace-separated tokens
    stringstream ss(cleaned);
    vector<string> tokens;
    string tok;

    while (ss >> tok) {
        tokens.push_back(tok);
    }

    if (tokens.empty()) {
        line.isComment = true;
        return;
    }

    string first = trim(tokens[0]);
    string check = first;

    // For format 4 instructions like +JSUB
    if (!check.empty() && check[0] == '+') {
        check = check.substr(1);
    }

    // If first token is *, opcode, or directive, then this line has no label
    if (check == "*" || optab.count(check) || is_directive(check)) {
        line.label = "";
        line.opcode = first;
        if (tokens.size() >= 2) line.operand = trim(tokens[1]);
    } else {
        line.label = first;
        if (tokens.size() >= 2) line.opcode = trim(tokens[1]);
        if (tokens.size() >= 3) line.operand = trim(tokens[2]);
    }
}

void LimitedAssembler::pass1() {
    ifstream in(sourceFilename);
    if (!in) {
        cerr << "Error: could not open " << sourceFilename << endl;
        return;
    }

    lines.clear();
    locctr = 0;
    startAddress = 0;
    programName = "";
    baseEnabled = false;
    baseRegister = 0;

    string raw;
    int lineNumber = 1;
    bool firstLine = true;

    while (getline(in, raw)) {
        SourceLine line;

        parse_line(raw, line, lineNumber);

        if (line.isComment) {
            line.address = locctr;
            lines.push_back(line);
            lineNumber++;
            continue;
        }

        if (firstLine && line.opcode == "START") {
            startAddress = hex_to_int(line.operand);
            locctr = startAddress;
            line.address = locctr;
            programName = line.label;

            if (!line.label.empty()) {
                symtab.insert_symbol(line.label, locctr, true);
            }

            lines.push_back(line);
            firstLine = false;
            lineNumber++;
            continue;
        }

        firstLine = false;
        line.address = locctr;

        line.label = trim(line.label);

        if (!line.label.empty() &&
            line.label != "*" &&
            !optab.count(line.label) &&
            !is_directive(line.label)) {
            if (!symtab.insert_symbol(line.label, locctr, false)) {
                cerr << "Duplicate symbol: " << line.label << endl;
            }
        }

        // If operand is a literal, add it to LITTAB
        if (!line.operand.empty() && line.operand[0] == '=') {
            littab.add_literal(line.operand);
        }

        string op = line.opcode;
        bool format4 = false;

        if (!op.empty() && op[0] == '+') {
            format4 = true;
            op = op.substr(1);
        }

        if (optab.count(op)) {
            FormatType fmt = optab[op].format;

            if (format4) {
                locctr += 4;
            } else if (fmt == F1) {
                locctr += 1;
            } else if (fmt == F2) {
                locctr += 2;
            } else {
                locctr += 3;
            }
        }
        else if (line.opcode == "WORD") {
            locctr += 3;
        }
        else if (line.opcode == "RESW") {
            locctr += 3 * stoi(line.operand);
        }
        else if (line.opcode == "RESB") {
            locctr += stoi(line.operand);
        }
        else if (line.opcode == "BYTE") {
            locctr += get_byte_length(line.operand);
        }
        else if (line.opcode == "LTORG" || line.opcode == "END") {
            // no immediate LOCCTR increment here
        }

        lines.push_back(line);

        // At LTORG or END, assign addresses to pending literals
        if (line.opcode == "LTORG" || line.opcode == "END") {
            vector<pair<int, Literal>> litLines;
            littab.assign_literals(locctr, litLines);

            for (auto& p : litLines) {
                SourceLine litLine;
                litLine.lineNumber = lineNumber;
                litLine.address = p.first;
                litLine.label = "*";
                litLine.opcode = p.second.name;
                litLine.operand = "";
                litLine.comment = "";
                litLine.objectCode = p.second.value;
                litLine.isComment = false;

                lines.push_back(litLine);
            }
        }

        lineNumber++;
    }

    programLength = locctr - startAddress;
    symtab.set_length(programLength);
}

void LimitedAssembler::pass2() {
    unordered_map<string, int> regNums = {
        {"A", 0}, {"X", 1}, {"L", 2}, {"B", 3},
        {"S", 4}, {"T", 5}, {"F", 6}, {"PC", 8}, {"SW", 9}
    };

    for (size_t i = 0; i < lines.size(); i++) {
        SourceLine& line = lines[i];

        if (line.isComment) continue;

        string op = line.opcode;
        bool format4 = false;

        if (!op.empty() && op[0] == '+') {
            format4 = true;
            op = op.substr(1);
        }

        // Literal lines already have object code from pass 1
        if (line.label == "*" && !line.objectCode.empty()) {
            continue;
        }

        if (line.opcode == "BASE") {
            SymtabInfo info;
            if (symtab.lookup_symbol(line.operand, info)) {
                baseRegister = info.address;
                baseEnabled = true;
            } else if (!line.operand.empty() && line.operand[0] == '=') {
                baseRegister = littab.get_address(line.operand);
                baseEnabled = true;
            }
            continue;
        }

        if (line.opcode == "NOBASE") {
            baseEnabled = false;
            continue;
        }

        if (line.opcode == "WORD") {
            line.objectCode = int_to_hex(stoi(line.operand), 6);
            continue;
        }

        if (line.opcode == "BYTE") {
            if (line.operand.size() >= 3 && line.operand[0] == 'C') {
                string data = line.operand.substr(2, line.operand.size() - 3);
                string result = "";
                for (char c : data) {
                    result += int_to_hex((int)(unsigned char)c, 2);
                }
                line.objectCode = result;
            }
            else if (line.operand.size() >= 3 && line.operand[0] == 'X') {
                line.objectCode = line.operand.substr(2, line.operand.size() - 3);
            }
            continue;
        }

        if (line.opcode == "RESW" || line.opcode == "RESB" ||
            line.opcode == "START" || line.opcode == "END" ||
            line.opcode == "LTORG") {
            continue;
        }

        if (!optab.count(op)) continue;

        OptabInfo info = optab[op];
        int opcode = info.opcode;

        if (info.format == F1 && !format4) {
            line.objectCode = int_to_hex(opcode, 2);
            continue;
        }

        if (info.format == F2 && !format4) {
            int r1 = 0, r2 = 0;

            if (!line.operand.empty()) {
                size_t comma = line.operand.find(',');
                if (comma == string::npos) {
                    if (regNums.count(line.operand)) r1 = regNums[line.operand];
                } else {
                    string left = line.operand.substr(0, comma);
                    string right = line.operand.substr(comma + 1);
                    if (regNums.count(left)) r1 = regNums[left];
                    if (regNums.count(right)) r2 = regNums[right];
                }
            }

            stringstream ss;
            ss << uppercase << hex << setw(2) << setfill('0') << opcode
               << setw(1) << r1
               << setw(1) << r2;
            line.objectCode = ss.str();
            continue;
        }

        int n = 1, iFlag = 1, x = 0, b = 0, p = 0, e = (format4 ? 1 : 0);
        string operand = line.operand;

        if (operand.empty() && op == "RSUB") {
            int opni = (opcode & 0xFC) | 0x03;
            line.objectCode = int_to_hex(opni, 2) + "0000";
            continue;
        }

        if (!operand.empty() && operand[0] == '#') {
            n = 0;
            iFlag = 1;
            operand = operand.substr(1);
        } else if (!operand.empty() && operand[0] == '@') {
            n = 1;
            iFlag = 0;
            operand = operand.substr(1);
        }

        size_t commaPos = operand.find(",X");
        if (commaPos != string::npos) {
            x = 1;
            operand = operand.substr(0, commaPos);
        }

        int targetAddress = 0;
        bool immediateConstant = !operand.empty() &&
                                 all_of(operand.begin(), operand.end(), ::isdigit);

        if (immediateConstant) {
            targetAddress = stoi(operand);
        }
        else if (!operand.empty() && operand[0] == '=') {
            targetAddress = littab.get_address(operand);
        }
        else {
            SymtabInfo symbolInfo;
            if (symtab.lookup_symbol(operand, symbolInfo)) {
                targetAddress = symbolInfo.address;
            } else {
                cerr << "Warning: undefined symbol '" << operand
                     << "' at line " << line.lineNumber << endl;
                targetAddress = 0;
            }
        }

        int opni = (opcode & 0xFC) | (n << 1) | iFlag;

        if (format4) {
            int flags = (x << 3) | (b << 2) | (p << 1) | e;
            stringstream ss;
            ss << uppercase << hex
               << setw(2) << setfill('0') << opni
               << setw(1) << flags
               << setw(5) << setfill('0') << (targetAddress & 0xFFFFF);
            line.objectCode = ss.str();
        } else {
            int disp = 0;

            if (immediateConstant) {
                disp = targetAddress;
                b = 0;
                p = 0;
            } else {
                int nextAddr = line.address + 3;
                int pcDisp = targetAddress - nextAddr;

                if (pcDisp >= -2048 && pcDisp <= 2047) {
                    disp = pcDisp & 0xFFF;
                    p = 1;
                    b = 0;
                } else if (baseEnabled) {
                    int baseDisp = targetAddress - baseRegister;
                    if (baseDisp >= 0 && baseDisp <= 4095) {
                        disp = baseDisp;
                        b = 1;
                        p = 0;
                    } else {
                        cerr << "Error: cannot fit displacement for operand '"
                             << operand << "' at line " << line.lineNumber << endl;
                        disp = 0;
                    }
                } else {
                    cerr << "Error: no valid PC/base displacement for operand '"
                         << operand << "' at line " << line.lineNumber << endl;
                    disp = 0;
                }
            }

            int flags = (x << 3) | (b << 2) | (p << 1) | e;
            stringstream ss;
            ss << uppercase << hex
               << setw(2) << setfill('0') << opni
               << setw(1) << flags
               << setw(3) << setfill('0') << (disp & 0xFFF);
            line.objectCode = ss.str();
        }
    }
}

void LimitedAssembler::assemble(const string& filename) {
    sourceFilename = filename;
    pass1();
    pass2();
    write_listing_file();
    write_symtab_file();

    cout << "Processed: " << filename << endl;
}

void LimitedAssembler::write_listing_file() const {
    string outName = sourceFilename + ".l";
    ofstream out(outName);

    if (!out) {
        cerr << "Error: could not create listing file " << outName << endl;
        return;
    }

    for (const auto& line : lines) {
        if (line.isComment) {
            out << "      " << line.comment << "\n";
            continue;
        }

        out << uppercase << hex
            << setw(4) << setfill('0') << line.address << "  "
            << left << setw(10) << line.label
            << setw(10) << line.opcode
            << setw(15) << line.operand
            << line.objectCode << "\n";
    }
}

void LimitedAssembler::write_symtab_file() const {
    string baseName = sourceFilename;
    if (baseName.size() >= 4 && baseName.substr(baseName.size() - 4) == ".sic") {
        baseName = baseName.substr(0, baseName.size() - 4);
    }

    string outName = baseName + ".st";

    // First write symbol section
    symtab.write_symtab(outName, programName.empty() ? "DEFAULT" : programName);

    // Then append literal section
    ofstream out(outName, ios::app);
    if (!out) {
        cerr << "Error: could not append to symtab file " << outName << endl;
        return;
    }

    out << "\n";
    out << "Literal Table\n";
    out << "Name  Operand  Address  Length:\n";
    out << "--------------------------------\n";

    for (const auto& lit : littab.get_all()) {
        string litName = lit.name;

        // convert =C'EOF' -> EOF for display
        if (litName.size() >= 4 && litName[0] == '=' &&
            (litName[1] == 'C' || litName[1] == 'X') &&
            litName[2] == '\'' && litName.back() == '\'') {
            litName = litName.substr(3, litName.size() - 4);
        }

        out << left << setw(6) << litName
            << setw(9) << lit.value
            << uppercase << hex << setw(7) << setfill('0') << lit.address
            << setfill(' ') << "  "
            << dec << lit.length
            << "\n";
    }
}
