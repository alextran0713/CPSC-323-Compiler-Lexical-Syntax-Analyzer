
#include "SymbolTable.h"
map <string, Symbol> symbolTable;
//string symbolTable[1000][3];

int memoryAddr = 5000;

void printTable(ostream &outfile)
{
	auto ittr = symbolTable.begin();
	auto ittrEnd = symbolTable.end();
	outfile << endl << endl;
	outfile << left << setw(20) << "\t\t\tSymbol table" << endl;
	for (int i = 0; i < 60; i++)
		outfile << "-";
	outfile << endl;
	outfile << left << setw(20) << "Identifier" 
		<< setw(20) << "Memory Location" 
		<< setw(20) << "Type" << endl;

	while (ittr != ittrEnd)
	{
		outfile << left 
			<< setw(20) << ittr->first 
			<< setw(20) << ittr->second.memoryLocation
			<< setw(20) << ittr->second.type << endl;
		ittr++;
	}
}

void printTable()
{
	auto ittr = symbolTable.begin();
	auto ittrEnd = symbolTable.end();
	cout << endl;
	cout << left << setw(20) << "\t\t\tSymbol table" << endl;
	for (int i = 0; i < 60; i++)
		cout << "-";
	cout << endl;
	cout << left 
		<< setw(20) << "Identifier" 
		<< setw(20) << "Memory Location" 
		<< setw(20) << "Type" 
		<< endl;
	
	cout << endl;
	while (ittr != ittrEnd)
	{
		cout << left 
			<< setw(20) << ittr->first 
			<< setw(20) << ittr->second.memoryLocation
			<< setw(20) << ittr->second.type << endl;
		ittr++;
	}
}

void insertItem(string type, string lexeme)
{
	if (inTable(lexeme))
	{
		//cerr << lexeme << " has been defined before.";
		return;
	}
	Symbol current = { lexeme, type, memoryAddr };
	symbolTable[lexeme] = current;
	memoryAddr++;
}

bool inTable(string lexeme)
{
	auto ittr = symbolTable.begin();
	auto ittrEnd = symbolTable.end();
	while (ittr != ittrEnd)
	{
		if (ittr->first == lexeme)
			return true;
		ittr++;
	}
	return false;
	//return symbolTable.find(lexeme) != symbolTable.end();
}

string get_address(string &lexeme)
{
	auto ittr = symbolTable.begin();
	auto ittrEnd = symbolTable.end();
	ittr = symbolTable.find(lexeme);

	while (ittr != ittrEnd)
	{
		if (ittr->first == lexeme)
		{
			lexeme = to_string(ittr->second.memoryLocation);
			return lexeme;
		}
			
		ittr++;
	}
	return "nil";
}

string get_type(string lexeme)
{
	return symbolTable.find(lexeme)->second.type;
}
