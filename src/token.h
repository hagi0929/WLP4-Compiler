#ifndef TOKEN_H
#define TOKEN_H

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Token {
 public:
  string kind;
  string lexeme;

  Token(string kind, string lexeme);
  int toInt();
};

#endif

