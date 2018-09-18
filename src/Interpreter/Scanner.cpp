/* scanner  Michael Pfeiffer  15.04.00 */

#include <math.h>
#include <ctype.h>
#include "Interpreter/Scanner.h"
/*
int16 ch;    // result after call of NextChar
int32 chPos; // and postition 
int16 tk;    // result after call of NextToken
int32 tkPos; // and position
Result value;
*/

// StringReader
StringReader::StringReader(const char *text = NULL) {
	this->text = text; i = 0;
}

void StringReader::SetString(const char *text = NULL) {
	this->text = text; i = 0;
}

void StringReader::Reset() {
	i = 0;
}

void StringReader::NextChar(int16 &ch, int32 &pos) {
	if (text == NULL) {
		ch = T_EOF; pos = 0;
	} else {
		pos = i;
		if (text[i] != 0) {
			ch = text[i]; i++;
		} else
			ch = T_EOF;
	}
}
 
// Scanner

double Scanner::Integer(int &digits) {
double n;
	n = 0; digits = 0;
	while (isdigit(ch)) {
		n = 10 * n + (ch-'0'); digits ++;
		reader->NextChar(ch, chPos);
	}
	return n;
}

void Scanner::Number() {
double n, m;
int d; bool sign = false;
	n = Integer(d);
	if (ch == '.') {
 		reader->NextChar(ch, chPos);
		m = Integer(d);
		n += m * pow(10, -d);
	}
	if (ch == 'e') {
		reader->NextChar(ch, chPos);
		if (ch == '-') { sign = true; reader->NextChar(ch, chPos); }
		else if (ch == '+') reader->NextChar(ch, chPos);
		m = Integer(d);
		if (sign) m = -m;
		n *= pow(10, m);
	}
	value.number = n;
	 
	tk = T_NUMBER;
}

void Scanner::Ident() {
int i = 0;
	while(isalnum(ch)) {
		value.ident[i] = (char)ch; i++;
		reader->NextChar(ch, chPos);
	}
	
	value.ident[i] = 0;
	if (strcmp(value.ident, "if") == 0) tk = T_IF;
	else if (strcmp(value.ident, "else") == 0) tk = T_ELSE;
	else if (strcmp(value.ident, "then") == 0) tk = T_THEN;	
	else tk = T_IDENT;
}

void Scanner::SkipWhiteSpaces() {
	while (isspace(ch)) reader->NextChar(ch, chPos);
}

#define EOL '\n'

void Scanner::SkipLineComment() {
	while (ch == '#') {
		do {
			reader->NextChar(ch, chPos);
		} while ((ch != T_EOF) && (ch != EOL));
		SkipWhiteSpaces();
	}
}

void Scanner::NextToken(int16 &_tk, int32 &_tkPos) {
	// dont read over end of file/stream 
	//if (ch == T_EOF) { tk = ch; return; }

	SkipWhiteSpaces();
	SkipLineComment();	

	tkPos = chPos;
	if (isdigit(ch)) Number();
	else if (isalpha(ch)) Ident();
	else {
		tk = ch;
		if (ch != T_EOF) reader->NextChar(ch, chPos);
		if (ch == '=') {
			switch (tk) {
			case '!': tk = T_NOT_EQUAL;
				break;
			case '=': tk = T_EQUAL;
				break;
			case '<': tk = T_LESS_EQUAL;
				break;
			case '>': tk = T_GREATER_EQUAL;
				break;
			default:
				_tk = tk; _tkPos = tkPos;
				return;
			}
			reader->NextChar(ch, chPos);
		}		
	}
	_tk = tk; _tkPos = tkPos;
}

Scanner::Scanner(Reader *reader = NULL) {
	this->reader = reader;
}

Scanner::~Scanner() {
	if (reader != NULL) {
		delete reader; reader = NULL;
	}
}

void Scanner::SetReader(Reader *reader = NULL) {
	if (this->reader != NULL) delete this->reader;
	this->reader = reader;
}

bool Scanner::Reset() {
	if (reader != NULL) {
		reader->Reset(); reader->NextChar(ch, chPos);
		return true;
	} else
		return false;
}
