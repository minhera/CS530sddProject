# README

Name: Minh Tran  
Class Account Username: cssc2552  
Name: Brian Madott
Class Account Username: cssc2537

Class Information: CS 530, Spring 2026  

## Assignment Information

Assignment: Project #2 – SIC/XE Limited Assembler


## File Manifest

main.cpp → Program entry point, handles command-line arguments  
limited_assembler.cpp → Contains Pass 1 and Pass 2 logic  
limited_assembler.h → Assembler class definition  
optab.cpp → Opcode table implementation  
optab.h → Opcode table definitions  
symtab.cpp → Symbol table implementation  
symtab.h → Symbol table definitions  
littab.cpp → Literal table implementation  
littab.h → Literal table definitions  
Makefile → Used to compile the project  
README → This file  

## Compile Instructions

On edoras / Linux (Read Makefile for device specification such as C++ 11):  
Make sure to have all files downloaded under the same folder.
On terminal:
1. type "make clean" without double quotations to clean everything
2. type "make" without double quotations to build the program

## Operating Instructions  

Run the assembler using:  
type "./lxe file_name.sic" without double quotations and change file_name to test files provided or extracted from somewhere else.

./lxe file.sic  
Example:  
./lxe P2sample.sic  

If no input files are provided, the program will display an error message and terminate.

## Input

.sic files containing SIC/XE assembly code

## Output

For each input file file.sic, the program generates:  

file.l → Listing file  
file.st → Symbol table file  


## Design Decisions

Two-pass assembler design:  
Pass 1 builds the symbol table and assigns addresses  
Pass 2 generates object code using the symbol table  
  
optab handles opcode information  
symtab handles symbol storage and lookup  
littab manages literals and delayed assignment  
  
Literals are stored during Pass 1 and assigned addresses at LTORG or END
  
Stringstream for Used to generate properly formatted hexadecimal object code.  
  
A vector is used to preserve insertion order of symbols for correct output formatting.  


## Extra Features

Support for format 1, 2, 3, and 4 instructions  
Support for immediate (#), indirect (@), and indexed (,X) addressing  
Implementation of PC-relative and base-relative addressing  
Literal table with handling of =C'...' and =X'...'  
Output files similar to textbook format


## Known Deficiencies / Bugs

There are some error handling but limited  
Assumes valid SIC/XE syntax in most cases  
Fixed formatting so the program is unable to output different format  
Only listing and symbol table so no full object program files  

## Lessons Learned

Understanding the importance of two-pass assembly and why symbol resolution requires it  
Learned how addressing modes (PC/base-relative) affect object code generation  
Improved debugging skills and error handling  
Learned the importance of clean and organized code


