# README

Name: Minh Tran  
Class Account Username: cssc2552  
Name:  
Class Account Username:  

Class Information: CS 530, Spring 2026  

## Assignment Information

Assignment: Project #2 – SIC/XE Limited Assembler


## File Manifest

* `main.cpp` → Program entry point, handles command-line arguments
* `limited_assembler.cpp` → Contains Pass 1 and Pass 2 logic
* `limited_assembler.h` → Assembler class definition
* `optab.cpp` → Opcode table implementation
* `optab.h` → Opcode table definitions
* `symtab.cpp` → Symbol table implementation
* `symtab.h` → Symbol table definitions
* `littab.cpp` → Literal table implementation
* `littab.h` → Literal table definitions
* `Makefile` → Used to compile the project
* `README` → This file
* `*.sic` → Test input files
* `*.l` → Listing output files (generated)
* `*.st` → Symbol table output files (generated)

---

## Compile Instructions

### On edoras / Linux:

```bash
make
```

### On Windows (PowerShell):

```bash
g++ main.cpp limited_assembler.cpp optab.cpp symtab.cpp littab.cpp -o lxe
```

---

## Operating Instructions

Run the assembler using:

```bash
./lxe file1.sic file2.sic
```

Example:

```bash
./lxe P2sample.sic
```

If no input files are provided, the program will display an error message and terminate.

### Input

* One or more `.sic` files containing SIC/XE assembly code

### Output

For each input file `example.sic`, the program generates:

* `example.l` → Listing file
* `example.st` → Symbol table file

---

## Design Decisions

* **Two-pass assembler design**
  Pass 1 builds the symbol table and assigns addresses.
  Pass 2 generates object code using the symbol table.

* **Separation of concerns**

  * `optab` handles opcode information
  * `symtab` handles symbol storage and lookup
  * `littab` manages literals and delayed assignment

* **Literal handling via LITTAB**
  Literals are stored during Pass 1 and assigned addresses at `LTORG` or `END`.

* **Use of vectors for ordering**
  A vector is used to preserve insertion order of symbols for correct output formatting.

* **Stringstream for formatting**
  Used to generate properly formatted hexadecimal object code.

---

## Extra Features

* Support for **format 1, 2, 3, and 4 instructions**
* Handling of **immediate (#), indirect (@), and indexed (,X) addressing**
* Implementation of **PC-relative and base-relative addressing**
* Literal table with proper handling of:

  * `=C'...'`
  * `=X'...'`
* Output formatting similar to textbook examples

---

## Known Deficiencies / Bugs

* Error handling is limited; the assembler may continue after encountering errors
* Some formatting differences may exist compared to the exact textbook output
* Does not generate full object program files (only listing and symbol table)
* Assumes valid SIC/XE syntax in most cases
* Limited support for complex expressions

---

## Lessons Learned

* Understanding the importance of **two-pass assembly** and why symbol resolution requires it
* Gained experience implementing **low-level instruction encoding**
* Learned how **addressing modes (PC/base-relative)** affect object code generation
* Improved debugging skills for **linker and parsing errors**
* Learned the importance of **clean code organization and modular design**
* Better understanding of how assemblers bridge human-readable code and machine instructions

---
