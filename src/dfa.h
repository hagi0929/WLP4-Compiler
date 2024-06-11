#ifndef DFA_H
#define DFA_H

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

const string STATES = ".STATES";
const string TRANSITIONS = ".TRANSITIONS";
const string INPUT = ".INPUT";

bool isChar(string s);
bool isRange(string s);
string squish(string s);
int hexToNum(char c);
char numToHex(int d);
string escape(string s);
string unescape(string s);

class DFA {
 public:
  DFA(istream& in);
  unordered_map<string, bool> states;
  unordered_map<string, unordered_map<char, string>> transitions;
  string getNextState(string currentState, char character);
  void addTransition(string fromState, string toState, vector<char> charVec);
  void addState(string fromState, bool accepting);
  bool isAcceptingState(string state);
 private:
  void DFAprint(istream& in);
};

#endif
