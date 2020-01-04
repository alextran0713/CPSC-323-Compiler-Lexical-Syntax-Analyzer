/*
   ************************************************************************
   CPSC 323 Fall 2019
   Project 3: Enhance SyntaxAnalyszer with IF, Else, While and Simple Table
   Date: December 16, 2019
   Group member: JOSEPH HOANG + ALEX TRAN
   ************************************************************************
*/

#include <iostream>
#include <cstdlib>
#include <cctype>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>

#include "SymbolTable.h"
using namespace std;

enum FSM_TRANSITIONS
{
	RESTART = 0,
	INTEGER = 1,
	REAL = 2,
	LETTER = 3,
	COMMENT = 4,
	OPERATOR = 5,
	SEPARATOR = 6,
	SPACE = 7,
	UNKNOWN = 8,
	$ = 9,
};

//						     int  real let  cmt  oper sep  sp   unk  $
int stateTable[][10] = {{0,  1,   2,   3,   4,   5,   6,   7,   8,   9},
/* STATE int */			{1,  1,   2,   0,   0,   0,   0,   0,   0,   0},
/* STATE real*/			{2,  2,   8,   0,   0,   0,   0,   0,   0,   8},
/* STATE letter */		{3,  3,   3,   3,   0,   0,   0,   0,   0,   3},
/* STATE comment */		{4,  4,   4,   4,   0,   4,   4,   4,   4,   4},
/* STATE operator */	{5,  0,   0,   0,   0,   0,   0,   0,   0,   0},
/* STATE separator */	{6,  0,   0,   0,   0,   0,   0,   0,   0,   0},
/* STATE space */		{7,  0,   0,   0,   0,   0,   0,   0,   0,   0},
/* STATE unknow */		{8,  8,   8,   8,   4,   8,   8,   0,   8,   8},
/* STATE $ */			{9,  3,   0,   3,   0,   0,   0,   0,   0,   3}, 
					   };

// struct to hold token information
struct Token
{
	string token;
	string lexemeName;
};

// function prototypes for lexical analyzer 
vector<Token> tokenVector;
vector<Token> lexer(string expression);
int getFSMCol(char currentChar);
string getToken(int prevState, string subExpression);
bool commentFlag = false;
bool Keyword(char* kwd);
bool Operator(char op);
bool Seperator(char sp);


// function prototypes for syntax analyzer 
/*---------------------------------------------------------------
|	Rule 1:			S -> A | If<Conditonal>	| Else | While		|
|	Rule 2:			A -> id = E									|
|	Rule 3: 		E -> TE'									|
|	Rule 4: 		T -> FT'									|
|	Rule 5: 		F -> P										|
|	Rule 6: 		P -> id										|
|	Rule 7: 		T'-> *FT' | /FT' | Epsilon					|
|	Rule 8:			E'-> +TE' | -TE' | Epsilon					|
|	Rule 9:			If -> (ERE) | E								|
|---------------------------------------------------------------|
*/

bool S(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool A(unsigned& index, vector <Token> tokenVector, ofstream& outfile);
bool E(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool T(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool Eprime(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool Tprime(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool F(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool P(unsigned& index, vector<Token> tokenVector, ofstream& outfile);

/*---------------------------------------------------------------
|	Part 3	Enhanced Function for While, If, Else				|
|---------------------------------------------------------------|
*/

struct instr_t {
	int step = 0;
	string instr = "";
	string address;
};

instr_t add;
vector <instr_t> instr_table;
int save = 0;

// function prototypes to improve syntax analyzer 
// We made a few changes to adjust on how to accept IF, Else, and While
// We will have a function called OperatorExpression which will check for Relop Operator
bool If(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool Else(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool While(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
bool OperatorExpression(unsigned& index, vector<Token> tokenVector, ofstream& outfile);
void gen_instr(string, string);
void print_instr_table(ostream &outfile);
//void back_patch(string jump_addr);

int main()
{
	ofstream outfile("output.txt");
	ifstream infile;
	string fileName = "";
	string expression = "";
	

	cout << "Finite State Machine project\n" << endl;
	cout << "Please enter file name: ";
	getline(cin, fileName);
	infile.open(fileName.c_str());

	//infile.open("input.txt");

	if (infile.fail())
	{
		cout << "*** ERROR - the file " << fileName << " cannot be found!";
		exit(1);
	}

	while (getline(infile, expression))
	{
		//tokenVector = lexer(expression);
		lexer(expression);

		// Part 3: create symbol table
		string type = "interger";
		for (unsigned x = 0; x < tokenVector.size(); ++x)
		{
			if (tokenVector[x].token == "KEYWORD")
			{
				if (tokenVector[x].lexemeName == "int")
				{
					type = "interger";
				}
				if (tokenVector[x].lexemeName == "real" || 
					tokenVector[x].lexemeName == "double")
				{
					type = "real";
				}
				if (tokenVector[x].lexemeName == "boolean" || 
					tokenVector[x].lexemeName == "bool")
				{
					type = "boolean";
				}
			}

			if (tokenVector[x].token == "IDENTIFIER")
				insertItem(type, tokenVector[x].lexemeName);
		}
	}

	// print rules
	unsigned index = 0;
	while (index < tokenVector.size()) {
		S(index, tokenVector, outfile);
		cout << "===============================================================================" << endl;
		index++;
	}

	printTable();
	printTable(outfile);
	print_instr_table(outfile);

	cout << endl << "Result also successfully saved to output.txt\n" << endl;
	infile.close();
	outfile.close();
	system("Pause");
	return 0;
}

/*
	 Parses the "expression" string using the FSM to
	 isolate each individual token and lexeme name in the expression.
*/
vector<Token> lexer(string expression)
{
	Token access;
	//vector<Token> tokenVector;
	char currentChar = ' ';
	int col = 0;
	int currentState = 0;
	int prevState = 0;
	string subExpression;

	for (unsigned x = 0; x < expression.length();)
	{
		currentChar = expression[x];
		if (currentChar == '!')
		{
			commentFlag = !commentFlag;
			if (x == expression.length() - 1)
				break;
			else
			{
				x++;
				continue;
			}
		}
		if (!commentFlag)
		{
			col = getFSMCol(currentChar);
			currentState = stateTable[currentState][col];
			if (currentState == 0)
			{
				if (prevState != SPACE)
				{
					access.lexemeName = subExpression;
					access.token = getToken(prevState, subExpression);
					tokenVector.push_back(access);
					subExpression = "";
				}
			}
			else
			{
				if (currentChar != ' ' && currentChar != '\t'
					&& currentChar != '!')
					subExpression += currentChar;
				x++;
			}
			prevState = currentState;
		}
		else
			x++;
	}
	// this ensures the last token gets saved when
	// we reach the end of the loop (if a valid token exists)
	if (currentState != SPACE && subExpression != "" && !commentFlag)
	{
		access.token = getToken(prevState, subExpression);
		prevState = currentState;
		access.lexemeName = subExpression;
		tokenVector.push_back(access);
	}
	return tokenVector;
}


//	Determines the state of the type of character being examined
int getFSMCol(char currentChar)
{
	if (isspace(currentChar))
		return SPACE;
	else if (isdigit(currentChar))
		return INTEGER;
	else if (currentChar == '.')
		return REAL;
	else if (currentChar == '$')
		return LETTER;
	else if (isalpha(currentChar))
		return LETTER;
	else if (currentChar == '!')
		return COMMENT;
	else if (Operator(currentChar))
		return OPERATOR;
	else if (Seperator(currentChar))
		return SEPARATOR;
	return UNKNOWN;
}

// Returns the string equivalent of an integer lexeme token type.
string getToken(int prevState, string subExpression)
{
	switch (prevState)
	{
	case INTEGER:
		return "INTEGER";
		break;
	case REAL:
		return "REAL";
		break;
	case OPERATOR:
		return "OPERATOR";
		break;
	case LETTER:
		char temp[10000];
		strcpy_s(temp, subExpression.c_str()); // copy & convert to char*
		if (Keyword(temp) == true)
			return "KEYWORD";
		else
			return "IDENTIFIER";
		break;
	case SEPARATOR:
		return "SEPARATOR";
		break;
	case UNKNOWN:
		return "UNKNOWN";
		break;
	default:
		return "ERROR";
		break;
	}
}

// Returns 'true' if the string is a KEYWORD
bool Keyword(char* kwd)
{
	if (!strcmp(kwd, "int") || !strcmp(kwd, "float") ||
		!strcmp(kwd, "bool") || !strcmp(kwd, "boolean") ||
		!strcmp(kwd, "do") || !strcmp(kwd, "doend") ||
		!strcmp(kwd, "for") || !strcmp(kwd, "forend") ||
		!strcmp(kwd, "output") || !strcmp(kwd, "input") ||
		!strcmp(kwd, "or") || !strcmp(kwd, "if") ||
		!strcmp(kwd, "else") || !strcmp(kwd, "ifend") ||
		!strcmp(kwd, "while") || !strcmp(kwd, "return") ||
		!strcmp(kwd, "whileend") || !strcmp(kwd, "then") ||
		!strcmp(kwd, "get") || !strcmp(kwd, "put") ||
		!strcmp(kwd, "endif") || !strcmp(kwd, "false") ||
		!strcmp(kwd, "true"))
		return (true);
	return (false);
}

// Returns 'true' if the character is an OPERATOR
bool Operator(char op)
{
	if (op == '+' || op == '-' || op == '*' ||
		op == '/' || op == '>' || op == '<' || op == '=')
		return (true);
	return (false);
}

// Returns 'true' if the character is a SEPERATOR
bool Seperator(char sp)
{
	if (sp == ')' || sp == '(' || sp == '{' ||
		sp == '}' || sp == ',' || sp == ';' ||
		sp == ':' || sp == '[' || sp == ']')
		return(true);
	return(false);
}

//Rule 1: S -> A | If<Conditonal>
bool S(unsigned& index, vector <Token> tokenVector, ofstream & outfile) {
	cout << "Token: " << tokenVector[index].token
		<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
	outfile << "Token: " << tokenVector[index].token
		<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
	if (tokenVector[index].token == "KEYWORD") {
		cout << "\t<Statement> -> <Compound> | <Assign> | If<Conditional> | Else | While<Expression>" << endl;
		outfile << "\t<Statement> -> <Compound> | <Assign> | If<Conditional> | Else | While<Expression> " << endl;
		if (tokenVector[index].lexemeName == "if") {
			If(index, tokenVector, outfile);
			return true;
		}
		else if (tokenVector[index].lexemeName == "Else") {
			Else(index, tokenVector, outfile);
			return true;
		}
		else if (tokenVector[index].lexemeName == "while") {
			While(index, tokenVector, outfile);
			return true;
		}
	}
	else if (tokenVector[index].token == "IDENTIFIER") {
		cout << "\t<Statement> -> <Compound> | <Assign> | <If> " << endl;
		outfile << "\t<Statement> -> <Compound> | <Assign> | <If> " << endl;
		save = index;
		A(index, tokenVector, outfile);

	}
	else {
		cerr << "\tExpected Identifier or Keyword " << endl;
		return false;
	}
	if (tokenVector[index].lexemeName == ";") {
		cout << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		cout << endl;
	}
	return true;
}

//Rule 2: A -> id = E
bool A(unsigned& index, vector <Token> tokenVector, ofstream & outfile) {
	cout << "\t<Assign> -> <Identifier> = <Expression>  " << endl;
	outfile << "\t<Assign> -> <Identifier> = <Expression>  " << endl;
	cout << endl;

	index++;
	if (tokenVector[index].lexemeName == "=") {
		cout << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;
		index++;
		////insert into instr_table
		//gen_instr("PUSHI", tokenVector[index].lexemeName);

		cout << endl;
		E(index, tokenVector, outfile);

		//insert into instr_table
		gen_instr("POPM", get_address(tokenVector[save].lexemeName));
		return true;
	}
	else {
		cerr << "Expected Operator =" << endl;
		return false;
	}
}

//Rule 3: E -> TE'
bool E(unsigned& index, vector <Token> tokenVector, ofstream & outfile) {
	cout << "Token: " << tokenVector[index].token
		<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
	outfile << "Token: " << tokenVector[index].token
		<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
	cout << "\t<Expression> -> <Term> <Expression Prime>" << endl;
	outfile << "\t<Expression> -> <Term> <Expression Prime>" << endl;

	T(index, tokenVector, outfile);
	Eprime(index, tokenVector, outfile);
	return true;
}

//Rule 4: T -> FT'
bool T(unsigned& index, vector <Token> tokenVector, ofstream & outfile) {
	cout << "\t<Term> -> <Factor> <Term Prime>" << endl;
	outfile << "\t<Term> -> <Factor> <Term Prime>" << endl;
	F(index, tokenVector, outfile);
	Tprime(index, tokenVector, outfile);
	return true;
}

//Rule 5: F -> P
bool F(unsigned& index, vector<Token> tokenVector, ofstream & outfile) {
	cout << "\t<Factor> -> <Primary>  " << endl;
	outfile << "\t<Factor> -> <Primary>" << endl;
	P(index, tokenVector, outfile);
	return true;
}

//Rule 6: P -> id
bool P(unsigned& index, vector<Token> tokenVector, ofstream & outfile) {
	cout << "\t<Primary> -> <Identifier> " << endl;
	outfile << "\t<Primary> -> <Identifier> " << endl;
	cout << endl;

	// insert instruction
	gen_instr("PUSHM", get_address(tokenVector[index].lexemeName));

	index++;
	return true;
}

//Rule 7: T' -> *FT' | /FT' | Epsilon
bool Tprime(unsigned& index, vector<Token> tokenVector, ofstream & outfile) {
	if (tokenVector[index].lexemeName == "*") {
		cout << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;

		cout << "\t<Empty> -> Epsilon" << endl;
		cout << "\t<Term Prime> -> * <Factor> <Term Prime> | / <Factor> <Term Prime> | <Empty>" << endl;

		outfile << "\t<Empty> -> Epsilon" << endl;
		outfile << "\t<Term Prime> -> * <Factor> <Term Prime> | / <Factor> <Term Prime> | <Empty>" << endl;
		cout << endl;

		index++;

		cout << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		F(index, tokenVector, outfile);

		// insert instruction
		gen_instr("MUL", get_address(tokenVector[index].lexemeName));

		Tprime(index, tokenVector, outfile);
	}
	else if (tokenVector[index].lexemeName == "/") {
		cout << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;

		cout << "\t<Empty> -> Epsilon" << endl;
		cout << "\t<Term Prime> -> * <Factor> <Term Prime> | / <Factor> <Term Prime> | <Empty>" << endl;

		outfile << "\t<Empty> -> Epsilon" << endl;
		outfile << "\t<Term Prime> -> * <Factor> <Term Prime> | / <Factor> <Term Prime> | <Empty>" << endl;
		cout << endl;

		index++;

		cout << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		F(index, tokenVector, outfile);

		// insert instruction
		gen_instr("DIV", get_address(tokenVector[index].lexemeName));

		Tprime(index, tokenVector, outfile);
	}
	return false;
}

//Rule 8: E' -> +TE' | -TE' | Epsilon
bool Eprime(unsigned& index, vector<Token> tokenVector, ofstream & outfile) {
	if (tokenVector[index].lexemeName == "+") {
		cout << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;
		cout << "\t<Empty> -> Epsilon" << endl;
		cout << "\t<Expression Prime> -> + <Term> <Expression Prime> |  <Empty>" << endl;

		outfile << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "\t<Empty> -> Epsilon" << endl;
		outfile << "\t<Expression Prime> -> + <Term> <Expression Prime> |  <Empty>" << endl;
		cout << endl;		

		index++;

		cout << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		T(index, tokenVector, outfile);

		// add instructions
		gen_instr("ADD", get_address(tokenVector[index].lexemeName));

		Eprime(index, tokenVector, outfile);
		return true;
	}
	else if (tokenVector[index].lexemeName == "-") {
		cout << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\t\tLexeme: " << tokenVector[index].lexemeName << endl;

		cout << "\t<Empty> -> Epsilon" << endl;
		cout << "\t<Expression Prime> ->  - <Term> <Expression Prime> | <Empty>" << endl;

		outfile << "\t<Empty> -> Epsilon" << endl;
		outfile << "\t<Expression Prime> ->  - <Term> <Expression Prime> | <Empty>" << endl;
		cout << endl;

		index++;

		cout << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		outfile << "Token: " << tokenVector[index].token
			<< "\tLexeme: " << tokenVector[index].lexemeName << endl;
		T(index, tokenVector, outfile);

		// insert instruction
		gen_instr("SUB", get_address(tokenVector[index].lexemeName));

		Eprime(index, tokenVector, outfile);
		return true;
	}
	return false;
}

/* ----- Part 3 ---- */

//Rule 9: If ->  (E R E) | E
bool If(unsigned& index, vector<Token> tokenVector, ofstream& outfile) {
	if (tokenVector[index].lexemeName == "if") {
		cout << "\tIf<(Conditional)> -> { <Statement> } " << endl;
		outfile << "\tIf<(Conditional)> -> { <Statement> } " << endl;
		cout << endl;

		// insert instruction
		//gen_instr("SUB", get_address(tokenVector[index].lexemeName));

		index++;
		if (tokenVector[index].lexemeName == "(") {
			index++;
			if (tokenVector[index].token == "IDENTIFIER") {
				index++;
				if (OperatorExpression(index, tokenVector, outfile)) {
					if (tokenVector[index].token == "IDENTIFIER") {
						index++;
						if (tokenVector[index].lexemeName == ")") {
							index++;
							if (tokenVector[index].lexemeName == "{") {
								index++;
								//Need to get next line of code 
								if (!S(index, tokenVector, outfile)) {
									return false;
								}
								index++;
								if (tokenVector[index].lexemeName == "}") {
									index;
									Else(index, tokenVector, outfile);
									return true;
								}
								index++;
							}
						}
						return true;
					}
				}
				else {
					return false;
				}
			}
		}
		else {
			cerr << "Expected Operater (" << endl;
			return false;
		}
	}
	return false;
}

//It check whether the (Conditional) is met.
bool OperatorExpression(unsigned& index, vector<Token> tokenVector, ofstream& outfile) {
	if (tokenVector[index].lexemeName == "==" ||
		tokenVector[index].lexemeName == "!=" ||
		tokenVector[index].lexemeName == "<" ||
		tokenVector[index].lexemeName == ">" ||
		tokenVector[index].lexemeName == "<=" ||
		tokenVector[index].lexemeName == ">=") {
		index++;
		return true;
	}
	else {
		return false;
	}
}

//Else will be call after we finish processing IF(Condition) and found Else{statement}
bool Else(unsigned& index, vector<Token> tokenVector, ofstream& outfile) {
	if (tokenVector[index].lexemeName == "Else") {
		cout << "Else { <Statements> }" << endl;
		outfile << "Else { <Statements> }" << endl;
		cout << endl;
		index++;
		if (tokenVector[index].lexemeName == "{") {
			//Need to get next line of code 
			index++;
			if (!S(index, tokenVector, outfile)) {
				return false;
			}
			index++;
			if (tokenVector[index].lexemeName == "}") {
				index++;
				return true;
			}
			index++;
		}
	}
	return false;
}

//While will be similar to If(Condition). While(Condition) will check for While 
bool While(unsigned& index, vector<Token> tokenVector, ofstream& outfile) {
	if (tokenVector[index].lexemeName == "while") {
		cout << "while <Expression> -> <Statement>" << endl;
		outfile << "while <Expression> -> <Statement>" << endl;
		cout << endl;

		// get address , then insert instruction
		string addr = get_address(tokenVector[index].lexemeName);
		gen_instr("LABEL", addr);

		index++;
		if (tokenVector[index].lexemeName == "(") {
			index++;
			if (tokenVector[index].token == "IDENTIFIER") {
				index++;
				if (OperatorExpression(index, tokenVector, outfile)) {
					if (tokenVector[index].token == "IDENTIFIER") {
						index++;
						if (tokenVector[index].lexemeName == ")") {

							// insert instruction
							gen_instr("JUMP", get_address(tokenVector[index].lexemeName));							
							//back_patch(addr);

							index++;
							if (tokenVector[index].lexemeName == "{") {
								index++;
								//Need to get next line of code 
								if (!S(index, tokenVector, outfile)) {
									return false;
								}
								index++;
								if (tokenVector[index].lexemeName == "}") {
									index;
									Else(index, tokenVector, outfile);
									return true;
								}
							}
						}
						return true;
					}
				}
				else {
					return false;
				}
			}
		}
		else {
			cerr << "Expected Operater (" << endl;
			return false;
		}
	}
	return false;
}

//Print function to print out the instruction table
void print_instr_table(ostream &outfile) 
{
	// on command line screen
	cout << endl << endl;
	cout << left << setw(20) << "\t\t\tInstructions table" << endl;
	for (int i = 0; i < 60; i++)
		cout << "-";
	cout << endl;
	cout << left
		<< setw(20) << "Step #"
		<< setw(20) << "Instructions"
		<< setw(20) << "Address" << endl;
	for (unsigned i = 0; i < instr_table.size(); i++) {
		cout << left << setw(20) << instr_table[i].step;
		cout << left << setw(20) << instr_table[i].instr;
		cout << left << setw(20) << instr_table[i].address << endl;
	}

	// on .txt file
	outfile << endl << endl;
	outfile << left << setw(20) << "\t\t\tInstructions table" << endl;
	for (int i = 0; i < 60; i++)
		outfile << "-";
	outfile << endl;
	outfile << left
		<< setw(20) << "Step #"
		<< setw(20) << "Instructions"
		<< setw(20) << "Address" << endl;
	for (unsigned i = 0; i < instr_table.size(); i++) {
		outfile << left << setw(20) << instr_table[i].step;
		outfile << left << setw(20) << instr_table[i].instr;
		outfile << left << setw(20) << instr_table[i].address << endl;
	}
}

/* instr_address  shows the current insturction address is global */
void gen_instr(string instr, string address){
	//instr_t add;
	add.step++;
	add.address = address;
	add.instr = instr;
	instr_table.push_back(add);
}

//void back_patch( string jump_addr) { 
//	string addr = pop_jumpstack(); 
//	instr_table.at(1).address = jump_addr;
//}