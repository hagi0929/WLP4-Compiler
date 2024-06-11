#include "token.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// INT	Keyword int: The string "int".
// WAIN	Keyword wain: The string "wain".
// IF	Keyword if: The string "if".
// ELSE	Keyword else: The string "else".
// WHILE	Keyword while: The string "while".
// PRINTLN	Keyword println: The string "println".
// RETURN	Keyword return: The string "return".
// NEW	Keyword new: The string "new".
// DELETE	Keyword delete: The string "delete".
// NULL	Keyword NULL: The string "NULL".
// else "ID"
// read this and complete it for me
Token::Token(string kind, string lexeme) {
  this->kind = kind;
  this->lexeme = lexeme;

  if (kind == "ALLID") {
    if (lexeme == "int") {
      this->kind = "INT";
    } else if (lexeme == "wain") {
      this->kind = "WAIN";
    } else if (lexeme == "if") {
      this->kind = "IF";
    } else if (lexeme == "else") {
      this->kind = "ELSE";
    } else if (lexeme == "while") {
      this->kind = "WHILE";
    } else if (lexeme == "println") {
      this->kind = "PRINTLN";
    } else if (lexeme == "return") {
      this->kind = "RETURN";
    } else if (lexeme == "new") {
      this->kind = "NEW";
    } else if (lexeme == "delete") {
      this->kind = "DELETE";
    } else if (lexeme == "NULL") {
      this->kind = "NULL";
    } else {
      this->kind = "ID";
    }
  }
  // check if it doesnt start with zero
  // numeric value of the decimal number must not exceed 2147483647
  if (kind == "NUM") {
    if (lexeme[0] == '0' && lexeme.length() > 1) {
      throw runtime_error("ERROR: 0");
    }
    if (stol(lexeme) > 2147483647) {
      throw runtime_error("ERROR: overflow");
    }
  }
}

int Token::toInt() {
  if (this->kind == "DECINT") {
    return stoi(this->lexeme);
  } else if (this->kind == "HEXINT") {
    return stoi(this->lexeme, 0, 16);
  } else if (this->kind == "REGISTER") {
    return stoi(this->lexeme.substr(1));
  }
  return 0;
}
