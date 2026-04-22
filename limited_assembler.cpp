#include "limited_assembler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

// Constructor initializes default values and loads opcode table
LimitedAssembler::LimitedAssembler(string &filename) {
    optab = construct_optab();   // Load opcode table from optab.cpp
    sourceFilename = "";
    programName = "";
    startAddress = 0;
    programLength = 0;
    locctr = 0;
    baseRegister = 0;
    baseEnabled = false;

    // Create name of listing and st file
    string baseName = filename;
    size_t dot = baseName.find(".");

    if (dot != string::npos){
        baseName = baseName.substr(0, dot);
    }

    lFilePath = baseName + ".l";
    stFilePath = baseName + ".st";
}

// Removes whitespace from both ends of a string
string LimitedAssembler::trim(const string& s) const {
    size_t start = s.find_first_not_of(" \t\r\n");  // first non-space
    if (start == string::npos){
        return ""; // string is all whitespace
    }

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

    if (cleaned[0] == '*'){
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
    // Try opening file
    ifstream in(sourceFilename);
    if (!in) {
        cerr << "Error: could not open " << sourceFilename << endl;
        return;
    }

    // Reset/Initialize variables at start of the program
    lines.clear();

    locctr = 0;
    startAddress = 0;
    programName = "";
    baseEnabled = false;
    baseRegister = 0;

    string raw;
    int lineNumber = 1;

    // Iterate through each line of source program and perform pass 1 operations
    while (getline(in, raw)) {
        SourceLine line;

        parse_line(raw, line, lineNumber);

        // Extract comments and store it for output later and skip to next line
        if (line.isComment) {
            line.address = locctr;
            lines.push_back(line);
            lineNumber++;
            continue;
        }

        // START assembler directive: Store address and insert label to symtab
        if (line.opcode == "START") {
            if (!line.operand.empty()){
                startAddress = hex_to_int(line.operand);
            }

            locctr = startAddress;
            line.address = locctr;
            programName = line.label;

            // Insert Control Section symbol (label of START)
            if (!line.label.empty()) {
                symtab.insert_symbol(line.label, locctr, true);
            }  

            lines.push_back(line);
            lineNumber++;
            continue;
        }

        // Dealt with START address, so time to check other possibilities
        line.address = locctr;

        line.label = trim(line.label);

        // Check if label can be put on symbol table (no duplicates, no opcode, no literals, no directives)
        if (!line.label.empty() &&
            line.label != "*" &&
            !optab.count(line.label) &&
            !is_directive(line.label)) {

            // Write and inform user if there are duplicate symbol
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

        // Check if the opcode is exteded or not
        if (!op.empty() && op[0] == '+') {
            format4 = true;
            op = op.substr(1);
        }

        // Check the type of Opcode it is, update locctr address accordingly
        if (optab.count(op)) {
            FormatType fmt = optab[op].format;

            // Check the format type and update locctr address accordingly
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
            // Check if the input operand is a digit
            if (line.operand.empty()){
                cerr << "Error: Need to be numeric value for RESW\n";
            } 
            else{
                bool isNumber = true;

                for (size_t i = 0; i < line.operand.size(); i++){
                    if (!isdigit(line.operand[i])){
                        isNumber = false;
                        break;
                    }
                }

                // Update locctr if it is a number
                if (!isNumber){
                    cerr << "Error: Need to be numeric value for RESW\n";
                }
                else{
                    locctr += 3 * stoi(line.operand);
                }
            }
        }
        else if (line.opcode == "RESB") {
            // Check if the input operand is a digit
            if (line.operand.empty()){
                cerr << "Error: Need to be numeric value for RESB\n";
            } 
            else{
                bool isNumber = true;

                for (size_t i = 0; i < line.operand.size(); i++){
                    if (!isdigit(line.operand[i])){
                        isNumber = false;
                        break;
                    }
                }

                // Update locctr if it is a number
                if (!isNumber){
                    cerr << "Error: Need to be numeric value for RESB\n";
                }
                else{
                    locctr += stoi(line.operand);
                }
            }
        }
        else if (line.opcode == "BYTE") {
            locctr += get_byte_length(line.operand);
        }
        else if (line.opcode == "LTORG" || line.opcode == "END") {
            // no immediate LOCCTR increment here because it will be assigned later
        }

        lines.push_back(line); // Add the line to the list

        // At LTORG or END, assign addresses to pending literals
        if (line.opcode == "LTORG" || line.opcode == "END") {
            vector<pair<int, Literal>> litLines;

            // Side Note for us; locctr is already updated inside assign literals function
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

    // Just in case if END was not there on source program, update program length
    if (programLength == 0){
        programLength = locctr - startAddress;
        symtab.set_length(programLength);
    }
}

void LimitedAssembler::pass2() {
    // Assign register values for machine instruction calculations
    unordered_map<string, int> regNums = {
        {"A", 0}, {"X", 1}, {"L", 2}, {"B", 3},
        {"S", 4}, {"T", 5}, {"F", 6}, {"PC", 8}, {"SW", 9}
    };

    // Iterate through each line again and perform pass 2
    for (size_t i = 0; i < lines.size(); i++) {
        SourceLine& line = lines[i];

        if (line.isComment){
            continue;
        } 

        string op = line.opcode;
        bool format4 = false;

        // Check if opcode is extended
        if (!op.empty() && op[0] == '+') {
            format4 = true;
            op = op.substr(1);
        }

        // Literal lines should have object code from pass 1
        if (line.label == "*" && !line.objectCode.empty()) {
            continue;
        }

        // Check if the program will use Base-relative addressing
        // before checking if PC-relative works
        if (line.opcode == "BASE") {
            SymtabInfo info;
            if (symtab.lookup_symbol(line.operand, info)) {
                // Search symbol table
                baseRegister = info.address;
                baseEnabled = true; // Allows base relative addressing mode calculations
            } else if (!line.operand.empty() && line.operand[0] == '=') {
                // Search up literal table
                baseRegister = littab.get_address(line.operand);
                baseEnabled = true;
            }
            continue;
        }

        // From Lecture: NOBASE is just setting base relative mode off
        if (line.opcode == "NOBASE") {
            baseEnabled = false;
            continue;
        }

        // WORD = 3 bytes, and 2 hex per byte -> 6 total hex digits
        if (line.opcode == "WORD") {
            line.objectCode = int_to_hex(stoi(line.operand), 6);
            continue;
        }

        if (line.opcode == "BYTE") {
            // Find out if opcode if C'EOF' or X'F1' stuff
            if (line.operand.size() >= 3 && line.operand[0] == 'C') {
                // Extract characters between '' and generate object code
                string data = line.operand.substr(2, line.operand.size() - 3);
                string result = "";
                for (char c : data) {
                    result += int_to_hex((int)(unsigned char)c, 2);
                }
                line.objectCode = result;
            }
            else if (line.operand.size() >= 3 && line.operand[0] == 'X') {
                // Side Note for us: F1 is already hex so no conversion is necessary
                line.objectCode = line.operand.substr(2, line.operand.size() - 3);
            }
            continue;
        }

        // These directives don't need object code, so skip them
        if (line.opcode == "RESW" || line.opcode == "RESB" ||
            line.opcode == "START" || line.opcode == "END" ||
            line.opcode == "LTORG") {
            continue;
        }

        if (!optab.count(op)){
            continue;
        }

        OptabInfo info = optab[op];
        int opcode = info.opcode;

        // Generate Object code for format 1 if it is format 1
        if (info.format == F1 && !format4) {
            line.objectCode = int_to_hex(opcode, 2);
            continue;
        }

        /**
         * Little notes: format 2 has 1 byte opcode and 1 byte for regs
         *               either 1 or 2 registers can be used.
         */
        // Generate Object code for format 2 if it is format 2
        if (info.format == F2 && !format4) {
            int r1 = 0, r2 = 0;

            // Update register values, which will be used for object code calculations
            if (!line.operand.empty()) {
                size_t comma = line.operand.find(',');

                // If theres no comma, then only 1 register is used
                if (comma == string::npos) {
                    if (regNums.count(line.operand)){
                        r1 = regNums[line.operand];
                    }
                } 
                else {
                    string left = line.operand.substr(0, comma);
                    string right = line.operand.substr(comma + 1);
                    if (regNums.count(left)){
                        r1 = regNums[left];  
                    } 
                    if (regNums.count(right)){
                        r2 = regNums[right];
                    } 
                }
            }

            stringstream ss;
            ss << uppercase << hex << setw(2) << setfill('0') << opcode
               << setw(1) << r1
               << setw(1) << r2;
            line.objectCode = ss.str();
            continue;
        }

        // Now format 1/2 is done, start initializing instruction flags 
        // (simple addressing mode by default)
        int n = 1; 
        int iFlag = 1; 
        int x = 0;
        int b = 0; 
        int p = 0; 
        int e = 0;
        
        if (format4){
            e = 1;
        }
        
        string operand = line.operand;

        if (operand.empty() && op == "RSUB") {
            // Masking and Addition will put last two bits (n and i) to 1 for simple addressing mode
            int opni = (opcode & 0xFC) | 0x03;
            line.objectCode = int_to_hex(opni, 2) + "0000";
            continue;
        }

        // Check Immediate or Indirect addressing mode 
        if (!operand.empty() && operand[0] == '#') {
            n = 0;
            iFlag = 1;
            operand = operand.substr(1); // Don't include the symbol
        } 
        else if (!operand.empty() && operand[0] == '@') {
            n = 1;
            iFlag = 0;
            operand = operand.substr(1);
        }

        // Check for Indexed addressing mode
        size_t commaPos = operand.find(",X");
        if (commaPos != string::npos) {
            x = 1;
            operand = operand.substr(0, commaPos);
        }

        // Start calculating target address
        int targetAddress = 0;

        // See if the operand is all digit
        bool immediateConstant = !operand.empty() &&
                                 all_of(operand.begin(), operand.end(), ::isdigit);

        // Search if operand is all digit, in littab, or in symtab, and update target address
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
            } 
            else {
                // Error output for debugging
                cerr << "Warning: undefined symbol '" << operand
                     << "' at line " << line.lineNumber << endl;
                targetAddress = 0;
            }
        }

        // Set up correct n and i flag bits
        int opni = (opcode & 0xFC) | (n << 1) | iFlag;

        // Check format 4, extended, generate object code
        if (format4) {
            int flags = (x << 3) | (b << 2) | (p << 1) | e;
            stringstream ss;
            ss << uppercase << hex
               << setw(2) << setfill('0') << opni
               << setw(1) << flags
               << setw(5) << setfill('0') << (targetAddress & 0xFFFFF);
            line.objectCode = ss.str();
            
            continue;
        } 
        
        // Format 1, 2, 4 are done -> Only format 3 left
        int disp = 0;

        if (immediateConstant) {
            // Check if displacement stays within the range
            if (targetAddress < 0 || targetAddress > 4095){
                cerr << "Error: immediate value at line "
                     << line.lineNumber << " is out of bounds\n";
            }

            disp = targetAddress & 0xFFF; // 0xFFF keeps displacement bits from full address
            b = 0;
            p = 0;
        } 
        else {
            int nextAddr = line.address + 3;
            int pcDisp = targetAddress - nextAddr;

            // Check bounds for displacement to see either PC-relative or Base relative works
            if (pcDisp >= -2048 && pcDisp <= 2047) {
                disp = pcDisp & 0xFFF;
                p = 1;
                b = 0;
            } 
            else if (baseEnabled) {
                int baseDisp = targetAddress - baseRegister;
                if (baseDisp >= 0 && baseDisp <= 4095) {
                    disp = baseDisp;
                    b = 1;
                    p = 0;
                } else {
                    cerr << "Error: Out of bounds displacement for operand '"
                            << operand << "' at line " << line.lineNumber << endl;
                    disp = 0;
                }
            } else {
                cerr << "Error: no valid PC/base displacement for operand '"
                        << operand << "' at line " << line.lineNumber << endl;
                disp = 0;
            }
        }

        // Grab the object code
        int flags = (x << 3) | (b << 2) | (p << 1) | e;
        stringstream ss;
        ss << uppercase << hex
            << setw(2) << setfill('0') << opni
            << setw(1) << flags
            << setw(3) << setfill('0') << (disp & 0xFFF);
        line.objectCode = ss.str();
    }
}

void LimitedAssembler::assemble(const string& filename) {
    sourceFilename = filename;
    pass1();
    pass2();
    write_listing_file();
    write_symtab_file();

    cout << "Source File processed: " << filename << endl;
}

void LimitedAssembler::print_listing_line(ofstream& out, const SourceLine& ln) const {
    int addrWidth    = 4;
    int labelWidth   = 8;
    int opcodeWidth  = 9;
    int operandWidth = 20;

    // If the line is a comment, just print as is while ignoring * comments
    if (ln.isComment) {
        string trimmed = ln.comment;
        size_t s = trimmed.find_first_not_of(" \t"); // Not include space

        // Skip if it shows * because it should be ignored
        if (s != string::npos && trimmed[s] == '*'){
            return;
        }

        out << ln.comment << "\n";
        return;
    }

    /**
     * Side note: All the hex, setw, setfill is done to match the output layout
     */

    if (ln.opcode == "START") {
        out << right << uppercase << hex << setw(addrWidth) << setfill('0') << ln.address
            << "    " << left << setfill(' ') << setw(labelWidth)   << ln.label
            << setw(opcodeWidth)  << ln.opcode << setw(operandWidth) << ln.operand << "\n";
        return;
    }

    if (ln.opcode == "END") {
        out << string(addrWidth + 4, ' ') << left << setfill(' ')
            << setw(labelWidth)   << "" << setw(opcodeWidth)  << ln.opcode
            << setw(operandWidth) << ln.operand << "\n";
        return;
    }

    if (ln.label == "*") {
        // Don't write down extra literals without object code
        if (ln.objectCode.empty()){
            return;
        }

        out << right << uppercase << hex << setw(addrWidth) << setfill('0') << ln.address
            << "    " << left << setfill(' ') << setw(labelWidth)   << "*" << setw(opcodeWidth)  << ln.opcode
            << setw(operandWidth) << "" << ln.objectCode << "\n";
        return;
    }

    bool isNotObjectCode =  (ln.opcode == "RESW" || ln.opcode == "RESB" || ln.opcode == "BASE" || ln.opcode == "NOBASE" ||
                            ln.opcode == "LTORG");

    out << right << uppercase << hex << setw(addrWidth) << setfill('0') << ln.address
        << "    " << left << setfill(' ') << setw(labelWidth)   << ln.label << setw(opcodeWidth)  << ln.opcode
        << setw(operandWidth) << ln.operand;

    if (!isNotObjectCode){
        out << ln.objectCode; 
    }
    out << "\n";
}

void LimitedAssembler::write_listing_file() const {
    string outName = lFilePath;
    ofstream out(outName);

    if (!out) {
        cerr << "Error: could not create listing file " << outName << endl;
        return;
    }

    // Find where END directive is for dealing with literals after END
    int endIndex = -1;
    for (int i = (int)lines.size() - 1; i >=0; i--){
        if (!lines[i].isComment && lines[i].opcode == "END"){
            endIndex = i;
            break;
        }
    }

    // Check if there are literals or anything that is generated after END directives
    // Potentially for assigning addresses
    vector<int> indexAfterEND;
    for (int i = endIndex + 1; i < (int)lines.size(); ++i){
        if (!lines[i].isComment && lines[i].label == "*"){
            indexAfterEND.push_back(i);
        }
    }

    // Print everything except END directive and literals that had address assigned afterwards
    for (int i = 0; i < (int)lines.size(); ++i){
        // Base Case: No need to print if at END
        if (i == endIndex){
            continue;
        }

        bool skipLine = false;
        
        // Find literal lines and mark it to ignore
        for (int j : indexAfterEND){
            if (i == j){
                skipLine = true;
                break;
            }
        }

        // Skip literals with no object code
        if (!lines[i].isComment && lines[i].label == "*" && lines[i].objectCode.empty()){
            skipLine = true;
        }
        if (skipLine){
            continue;
        }

        print_listing_line(out, lines[i]);
    }

    // Print END and literals
    for (int i : indexAfterEND){
        print_listing_line(out, lines[i]);
    }

    if (endIndex >= 0){
        print_listing_line(out, lines[endIndex]);
    }
}

void LimitedAssembler::write_symtab_file() const {
    string outName = stFilePath;

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

        // convert =C'EOF' -> EOF to match the expected output format
        if (litName.size() >= 4 && litName[0] == '=' && (litName[1] == 'C' || litName[1] == 'X') &&
            litName[2] == '\'' && litName.back() == '\'') {
            litName = litName.substr(3, litName.size() - 4);
        }

        out << left << setw(6) << litName
            << setw(9) << lit.value
            << uppercase << hex << setw(7) << setfill(' ') << lit.address
            << setfill(' ') << "  "
            << dec << lit.length
            << "\n";
    }
}
