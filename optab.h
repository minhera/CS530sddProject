/**
 * Student 1 Name: Brian Madott
 * Student 1 Class account username: cssc2537
 * Student 2 Name: Minh Tran
 * Student 2 Class account username: cssc2552
 * Class: CS 530, Spring 2026
 * Assignment: Assignment #2 - Final SDD and Full Package
 * File Name: optab.h
 */

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