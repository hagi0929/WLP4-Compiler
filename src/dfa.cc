#include "dfa.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

DFA::DFA(istream& in) { DFAprint(in); }
string DFA::getNextState(string currentState, char character) {
  auto stateIt = this->transitions.find(currentState);
  if (stateIt != this->transitions.end()) {
    auto charIt = stateIt->second.find(character);
    if (charIt != stateIt->second.end()) {
      return charIt->second;
    }
  }
  return "";
}
void DFA::addTransition(string fromState, string toState,
                        vector<char> charVec) {
  if (this->transitions.find(fromState) == this->transitions.end()) {
    unordered_map<char, string> temp;
    this->transitions.insert(make_pair(fromState, temp));
  }
  for (char c : charVec) {
    this->transitions.find(fromState)->second.insert(make_pair(c, toState));
  }
}
void DFA::addState(string fromState, bool accepting) {
  this->states.insert(make_pair(fromState, accepting));
}
bool DFA::isAcceptingState(string state) {
  auto stateIt = states.find(state);
  if (stateIt != states.end()) {
    return stateIt->second;
  }
  return false;
}

void DFA::DFAprint(istream& in) {
  string s;
  while (true) {
    if (!(getline(in, s))) {
      throw runtime_error("Expected " + STATES + ", but found end of input.");
    }
    s = squish(s);
    if (s == STATES) {
      break;
    }
    if (!s.empty()) {
      throw runtime_error("Expected " + STATES + ", but found: " + s);
    }
  }
  bool initial = true;
  while (true) {
    if (!(in >> s)) {
      throw runtime_error("Unexpected end of input while reading state set: " +
                          TRANSITIONS + "not found.");
    }
    if (s == TRANSITIONS) {
      break;
    }
    bool accepting = false;
    if (s.back() == '!' && s.length() > 1) {
      accepting = true;
      s.pop_back();
    }
    this->addState(s, accepting);
    initial = false;
  }
  getline(in, s);
  while (true) {
    if (!(getline(in, s))) {
      break;
    }
    s = squish(s);
    if (s == INPUT) {
      break;
    }
    string lineStr = s;
    stringstream line(lineStr);
    vector<string> lineVec;
    while (line >> s) {
      lineVec.push_back(s);
    }
    if (lineVec.empty()) {
      continue;
    }
    if (lineVec.size() < 3) {
      throw runtime_error("Incomplete transition line: " + lineStr);
    }
    string fromState = lineVec.front();
    string toState = lineVec.back();
    vector<char> charVec;
    for (int i = 1; i < lineVec.size() - 1; ++i) {
      string charOrRange = escape(lineVec[i]);
      if (isChar(charOrRange)) {
        char c = charOrRange[0];
        if (c < 0 || c > 127) {
          throw runtime_error(
              "Invalid (non-ASCII) character in transition line: " + lineStr +
              "\n" + "Character " + unescape(string(1, c)) +
              " is outside ASCII range");
        }
        charVec.push_back(c);
      } else if (isRange(charOrRange)) {
        for (char c = charOrRange[0];
             charOrRange[0] <= c && c <= charOrRange[2]; ++c) {
          charVec.push_back(c);
        }
      } else {
        throw runtime_error("Expected character or range, but found " +
                            charOrRange + " in transition line: " + lineStr);
      }
    }
    this->addTransition(fromState, toState, charVec);
  }
}

bool isChar(string s) { return s.length() == 1; }

bool isRange(string s) { return s.length() == 3 && s[1] == '-'; }

string squish(string s) {
  stringstream ss(s);
  string token;
  string result;
  string space = "";
  while (ss >> token) {
    result += space;
    result += token;
    space = " ";
  }
  return result;
}

int hexToNum(char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return 10 + (c - 'a');
  } else if ('A' <= c && c <= 'F') {
    return 10 + (c - 'A');
  }
  throw runtime_error("Invalid hex digit!");
}

char numToHex(int d) { return (d < 10 ? d + '0' : d - 10 + 'A'); }

string escape(string s) {
  string p;
  for (int i = 0; i < s.length(); ++i) {
    if (s[i] == '\\' && i + 1 < s.length()) {
      char c = s[i + 1];
      i = i + 1;
      if (c == 's') {
        p += ' ';
      } else if (c == 'n') {
        p += '\n';
      } else if (c == 'r') {
        p += '\r';
      } else if (c == 't') {
        p += '\t';
      } else if (c == 'x') {
        if (i + 2 < s.length() && isxdigit(s[i + 1]) && isxdigit(s[i + 2])) {
          if (hexToNum(s[i + 1]) > 8) {
            throw runtime_error("Invalid escape sequence \\x" +
                                string(1, s[i + 1]) + string(1, s[i + 2]) +
                                ": not in ASCII range (0x00 to 0x7F)");
          }
          char code = hexToNum(s[i + 1]) * 16 + hexToNum(s[i + 2]);
          p += code;
          i = i + 2;
        } else {
          p += c;
        }
      } else if (isgraph(c)) {
        p += c;
      } else {
        p += s[i];
      }
    } else {
      p += s[i];
    }
  }
  return p;
}

string unescape(string s) {
  string p;
  for (int i = 0; i < s.length(); ++i) {
    char c = s[i];
    if (c == ' ') {
      p += "\\s";
    } else if (c == '\n') {
      p += "\\n";
    } else if (c == '\r') {
      p += "\\r";
    } else if (c == '\t') {
      p += "\\t";
    } else if (!isgraph(c)) {
      string hex = "\\x";
      p += hex + numToHex((unsigned char)c / 16) +
           numToHex((unsigned char)c % 16);
    } else {
      p += c;
    }
  }
  return p;
}
