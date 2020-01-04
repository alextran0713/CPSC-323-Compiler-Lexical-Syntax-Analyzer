#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <map>
#include <cstdlib>
#include <cctype>
#include <iomanip>

using namespace std;

void printTable(ostream &outfile);
void printTable();
void insertItem(string type, string lexeme);
bool inTable(string lexeme);
string get_address(string &lexeme);
string get_type(string lexeme);

struct Symbol
{
	string lexeme;	// identifier
	string type;	// int
	int memoryLocation;
	//int lenth = 0;
};