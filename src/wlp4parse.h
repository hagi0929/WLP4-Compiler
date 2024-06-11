#include <bitset>
#include <cctype>
#include <cstdio>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "dfa.h"
#include "token.h"
#include "wlp4data.h"

using namespace std;

stringstream out{""};

class Node {
 public:
  string value = "";
  vector<unique_ptr<Node>> children;
  int depth = 0;

  Node(string value, int depth) : value{value}, children{}, depth{depth} {}

  void addChild(unique_ptr<Node> node) { children.push_back(move(node)); }
  string getValue() { return value; }
  void printTree(string prefix = "", string childPrefix = "") {
    out << prefix << value << endl;

    for (size_t i = 0; i < children.size(); ++i) {
      bool isLast = (i == (children.size() - 1));
      children[i]->printTree();
    }
  }
};

deque<Token> tokenHelper(DFA& rcvDfa, istream& in) {
  char c;
  string currentState = "start";
  string nextState;
  string token = "";
  deque<Token> tokens;
  while (in.get(c)) {
    nextState = rcvDfa.getNextState(currentState, c);
    if (!nextState.empty()) {
      token += c;
      currentState = nextState;
    } else {
      //       if (t.kind == "NEWLINE") {
      //   cout << "NEWLINE" << endl;
      // } else if (t.kind == "?WHITESPACE") {
      // } else if (t.kind == "?COMMENT") {

      if (rcvDfa.isAcceptingState(currentState)) {
        tokens.push_back(Token(currentState, token));
        currentState = "start";
        token = "";
        in.putback(c);
      } else {
        throw runtime_error("ERROR: 1" + currentState + " " + c);
      }
    }
  }

  if (rcvDfa.isAcceptingState(currentState)) {
    tokens.push_back(Token(currentState, token));
  } else {
    throw runtime_error("ERROR: 2" + currentState + " " + c);
  }
  return tokens;
};

struct PairHash {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2>& pair) const {
    auto hash1 = std::hash<T1>{}(pair.first);
    auto hash2 = std::hash<T2>{}(pair.second);
    return hash1 ^ hash2;
  }
};

class wlp4parse {
 public:
  deque<Token> tokens;
  deque<pair<string, vector<string>>> cfg;
  unordered_map<pair<int, string>, int, PairHash> transitions;
  unordered_map<pair<int, string>, int, PairHash> reductions;
  vector<int> stateStack;
  deque<unique_ptr<Node>> nodeStack;
  unique_ptr<Node> parseTree;

  wlp4parse(deque<Token>& tokens) {
    // cfg
    for (auto t : tokens) {
      if (t.kind == "NEWLINE") {
      } else if (t.kind == "?WHITESPACE") {
      } else if (t.kind == "?COMMENT") {
      } else {
        this->tokens.emplace_back(t);
      }
    }
    istringstream c(WLP4_CFG);
    string line;
    while (getline(c, line)) {
      if (line == ".CFG") {
        continue;
      }

      stringstream ss(line);
      string lhs;
      string rhs;
      ss >> lhs;
      vector<string> rhsVec;
      while (ss >> rhs) {
        if (rhs == ".EMPTY") {
          continue;
        }

        rhsVec.emplace_back(rhs);
      }
      cfg.emplace_back(make_pair(lhs, rhsVec));
    }
    // transitions
    istringstream s(WLP4_TRANSITIONS);
    while (getline(s, line)) {
      // cout << line << endl;
      if (line == ".TRANSITIONS") {
        continue;
      }
      stringstream ss(line);
      string state;
      string symbol;
      string action;
      ss >> state >> symbol >> action;
      transitions[make_pair(stoi(state), symbol)] = stoi(action);
      // cout << "transition: " << state << " " << symbol << " " << action <<
      // endl;
    }
    // reductions
    istringstream r(WLP4_REDUCTIONS);
    // cout << WLP4_REDUCTIONS << endl;
    while (getline(r, line)) {
      if (line == ".REDUCTIONS") {
        continue;
      }

      stringstream ss(line);
      string state;
      string symbol;
      string action;
      ss >> state >> action >> symbol;
      reductions[make_pair(stoi(state), symbol)] = stoi(action);
      // cout << "reduc: " << state << " " << symbol << " " << action << endl;
    }
  };

  void reduceTrees(int rule) {
    // cout << "reducing" << to_string(rule) << endl;
    pair<string, vector<string>> ruleObj = cfg[rule];
    // cout << "debugging " << ruleObj.first << endl;
    string txt = ruleObj.first;
    if (ruleObj.second.size() == 0) {
      txt += " .EMPTY";
    } else {
      for (auto t : ruleObj.second) {
        txt += " " + t;
      }
    }
    auto newNode = make_unique<Node>(txt, 0);
    int len = ruleObj.second.size();

    for (int i = 0; i < len; ++i) {
      newNode->addChild(move(nodeStack[nodeStack.size() - len + i]));
    }

    nodeStack.erase(nodeStack.end() - len, nodeStack.end());
    nodeStack.emplace_back(move(newNode));
  }
  void reduceStates(int rule) {
    // cout << "reducing" << to_string(rule) << endl;
    pair<string, vector<string>> ruleObj = cfg[rule];

    int len = ruleObj.second.size();
    while (len--) {
      stateStack.pop_back();
    }

    int currentState = stateStack.back();
    string lhs = ruleObj.first;
    auto it = transitions.find(make_pair(currentState, lhs));
    if (it != transitions.end()) {
      stateStack.emplace_back(it->second);
    } else {
      throw runtime_error(to_string(rule) + ": state " +
                          to_string(currentState) + " and symbol " + lhs);
    }
  }

  void shift() {
    int currentState = stateStack.back();
    Token currentToken = tokens.front();
    string symbol = currentToken.kind;

    // cout << "shifting: " << to_string(stateStack.back()) << endl;
    // cout << "symbol: " << symbol << endl;

    auto transitionIt = transitions.find(make_pair(currentState, symbol));
    if (transitionIt != transitions.end()) {
      int nextState = transitionIt->second;
      stateStack.push_back(nextState);
      // cout << "debugging " << currentToken.kind << endl;
      auto newNode = make_unique<Node>(currentToken.kind + " " +currentToken.lexeme, 0);
      nodeStack.emplace_back(move(newNode));

      tokens.pop_front();
    } else {
      throw runtime_error("notnotnot found for state " +
                          to_string(currentState) + " and symbol " + symbol);
    }
  }
  void parse() {
    stateStack.push_back(0);
    tokens.emplace_front(Token("BOF", "BOF"));
    tokens.emplace_back(Token("EOF", "EOF"));
    // print token
    for (auto t : tokens) {
      // cout << t.kind << " " << t.lexeme << endl;
    }
    while (!tokens.empty()) {
      int currentState = stateStack.back();
      string symbol = tokens.front().kind;
      // cout << "currentState: " << to_string(currentState) << ", " << symbol
      //      << endl;
      auto it = reductions.find(make_pair(currentState, symbol));
      if (it != reductions.end()) {
        reduceTrees(it->second);
        reduceStates(it->second);
      } else {
        shift();
      }
    }
    parseTree = std::make_unique<Node>("start BOF procedures EOF", 0);
    for (auto& t : nodeStack) {
      parseTree->addChild(std::move(t));
    }

    nodeStack.clear();
    parseTree->printTree();
  }
};

