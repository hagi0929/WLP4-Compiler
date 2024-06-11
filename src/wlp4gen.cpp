#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "wlp4parse.h"
using namespace std;

// to MIPS instructions

int stackCount = 0;
int delcount = 0;

void add(int d, int s, int t) {
  cout << "add $" << d << ", $" << s << ", $" << t << endl;
}

void sub(int d, int s, int t) {
  cout << "sub $" << d << ", $" << s << ", $" << t << endl;
}

void mult(int d, int s) { cout << "mult $" << d << ", $" << s << endl; }

void sdiv(int d, int s) { cout << "div $" << d << ", $" << s << endl; }

void Label(string name) { std::cout << name + ":\n"; }

void mfhi(int d) { cout << "mfhi $" << d << endl; }

void mflo(int d) { cout << "mflo $" << d << endl; }

void lis(int d) { cout << "lis $" << d << endl; }

void slt(int d, int s, int t) {
  cout << "slt $" << d << ", $" << s << ", $" << t << endl;
}

void sltu(int d, int s, int t) {
  cout << "sltu $" << d << ", $" << s << ", $" << t << endl;
}

void jr(int s) { cout << "jr $" << s << endl; }

void jalr(int s) { cout << "jalr $" << s << endl; }

void beq(int s, int t, string i) {
  cout << "beq $" << s << ", $" << t << ", " << i << endl;
}

void bne(int s, int t, string i) {
  cout << "bne $" << s << ", $" << t << ", " << i << endl;
}

void lw(int t, int i, int s) {
  cout << "lw $" << t << ", " << i << "($" << s << ")" << endl;
}

void sw(int t, int i, int s) {
  cout << "sw $" << t << ", " << i << "($" << s << ")" << endl;
}

void lw(int t, string i, int s) {
  cout << "lw $" << t << ", " << i << "($" << s << ")" << endl;
}

void sw(int t, string i, int s) {
  cout << "sw $" << t << ", " << i << "($" << s << ")" << endl;
}

void word(int i) { cout << ".word " << i << endl; }
void word(string i) { cout << ".word " << i << endl; }

class exampleError : public exception {
 private:
  char* message;

 public:
  exampleError(char* msg) : message(msg) {}
  ostream& operator<<(ostream& in) {
    in << message;
    return in;
  }
};

const string CFG = ".CFG";
const string DERIVATION = ".DERIVATION";

class VariableTable;
class Node1 {
 public:
  string type = "";
  string rule;
  string lhs;
  vector<string> rhs;
  vector<shared_ptr<Node1>> children;

  Node1(const string& wholeline) : rule(wholeline) {
    stringstream iss{wholeline};
    iss >> lhs;
    string temp;
    while (iss >> temp) {
      rhs.push_back(temp);
    }
  }

  void annotateType(VariableTable& localSymbolTable);

  void addChild(shared_ptr<Node1> node) {
    if (node) {
      children.push_back(node);
    }
  }

  shared_ptr<Node1> getChild(const string& str, size_t index = 0) {
    if (index < rhs.size() && rhs[index] == str && index < children.size()) {
      return children[index];
    }
    return nullptr;
  }

  void printTree(const string& prefix = "", const string& childPrefix = "") {
    cout << prefix << rule << " : " << type << endl;
    for (size_t i = 0; i < children.size(); ++i) {
      bool isLast = (i == (children.size() - 1));
      children[i]->printTree(childPrefix + (isLast ? "╰─" : "├─"),
                             childPrefix + (isLast ? "  " : "│ "));
    }
  }
};

class Variable {
 public:
  string type;
  string name;

  Variable(shared_ptr<Node1> node) {
    if (node && node->lhs == "dcl") {
      auto typeNode1 = node->getChild("type", 0);
      if (typeNode1) {
        if (typeNode1->getChild("INT", 0)) {
          if (typeNode1->getChild("STAR", 1)) {
            type = "int*";
          } else {
            type = "int";
          }
        } else {
          throw runtime_error("Invalid type in declaration");
        }
        auto idNode1 = node->getChild("ID", 1);
        if (idNode1) {
          name = idNode1->rhs[0];
        } else {
          throw runtime_error("No ID in declaration");
        }
      } else {
        throw runtime_error("Invalid declaration node");
      }
    } else {
      throw runtime_error("Node1 is not a declaration");
    }
  }
};

class VariableTable {
 public:
  unordered_map<string, shared_ptr<Variable>> table;

  void Add(shared_ptr<Variable> var) {
    if (var && table.find(var->name) == table.end()) {
      table[var->name] = var;
    } else {
      throw runtime_error("Duplicate variable declaration or null variable");
    }
  }

  shared_ptr<Variable> getVariable(const string& name) {
    auto it = table.find(name);
    if (it != table.end()) {
      return it->second;
    }
    return nullptr;
  }
};

class Procedure {
 public:
  string name;
  vector<Variable> signature;
  VariableTable localSymbolTable;
  shared_ptr<Node1> dcls;
  shared_ptr<Node1> statements;
  shared_ptr<Node1> expr;
  Procedure(shared_ptr<Node1> node) {
    if (node->lhs == "main") {
      name = "wain";
      shared_ptr<Node1> dclNode11 = node->getChild("dcl", 3);
      shared_ptr<Node1> dclNode12 = node->getChild("dcl", 5);
      if (dclNode11 && dclNode12) {
        signature.emplace_back(Variable(dclNode11));
        signature.emplace_back(Variable(dclNode12));
      } else {
        throw runtime_error("param error");
      }
      dcls = node->getChild("dcls", 8);
      statements = node->getChild("statements", 9);
      expr = node->getChild("expr", 11);
      if (signature[1].type != "int") {
        throw runtime_error("wain param type error");
      }
      localSymbolTable.Add(make_shared<Variable>(signature[0]));
      localSymbolTable.Add(make_shared<Variable>(signature[1]));

    } else if (node->lhs == "procedure") {
      name = node->getChild("ID", 1)->rhs[0];
      auto paramNode1 = node->getChild("params", 3)->getChild("paramlist", 0);
      while (paramNode1 && paramNode1->lhs == "paramlist") {
        signature.emplace_back(Variable(paramNode1->getChild("dcl", 0)));
        localSymbolTable.Add(make_shared<Variable>(signature.back()));

        paramNode1 = paramNode1->getChild("paramlist", 2);
      }
      dcls = node->getChild("dcls", 6);
      statements = node->getChild("statements", 7);
      expr = node->getChild("expr", 9);

    } else {
      throw runtime_error("not main or procedure");
    }
    while (dcls->getChild("dcl", 1)) {
      auto temp = make_shared<Variable>(dcls->getChild("dcl", 1));
      if (temp->type != "int" && dcls->getChild("NUM", 3)) {
        throw runtime_error("dcl type error");
      }
      if (temp->type != "int*" && dcls->getChild("NULL", 3)) {
        throw runtime_error("dcl type error");
      }
      localSymbolTable.Add(temp);

      dcls = dcls->getChild("dcls", 0);
    }
    expr->annotateType(localSymbolTable);

    if (expr->type != "int") {
      throw runtime_error("return type error");
    }
    checkStatment(statements);
    if (expr->type != "int") {
      throw runtime_error("return type error");
    }
  }
  void IDChecker(shared_ptr<Node1>& node,
                 unordered_map<string, shared_ptr<Procedure>>& procedureTable) {
    if (!node) return;
    if (node->lhs == "paramlist") {
      return;
    }
    if (node->lhs == "statement" && node->getChild("lvalue", 0) &&
        node->getChild("BECOMES", 1) && node->getChild("expr", 2)) {
      node->annotateType(localSymbolTable);
      if (node->getChild("lvalue", 0)->type !=
          node->getChild("expr", 2)->type) {
        throw runtime_error("Type mismatch for assignment");
      }
    }
    // Check if node is an ID and exists in the symbol table
    if (node->lhs == "factor" && node->getChild("ID", 0) &&
        node->getChild("LPAREN", 1)) {
      if (node->rhs.size() == 4) {
        auto paramNode1 = node->getChild("arglist", 2);
        string funcName = node->getChild("ID", 0)->rhs[0];
        if (procedureTable.find(funcName) != procedureTable.end()) {
          checkProcedureCall(procedureTable[funcName], paramNode1);
        } else {
          throw runtime_error("Undeclared identifier: ");
        }
        // empty param
      } else {
        string funcName = node->getChild("ID", 0)->rhs[0];
        if (procedureTable.find(funcName) != procedureTable.end()) {
          checkProcedureCall(procedureTable[funcName], nullptr);
        } else {
          throw runtime_error("Undeclared identifier: ");
        }
      }

    } else if (node->lhs == "ID") {
      string id = node->rhs[0];
      auto var = localSymbolTable.getVariable(id);
      node->annotateType(localSymbolTable);
      if (var) {
        if (var->type != node->type) {
          throw runtime_error("Type mismatch for ID: " + id);
        }
      } else {
        if (node->type != "int") {
          throw runtime_error("11Type mismatch for ID: " + id + "|");
        }
      }
    }

    // Recursively check all children nodes
    for (auto& child : node->children) {
      if (node->lhs == "main" && child->lhs == "dcl") {
        continue;
      }
      IDChecker(child, procedureTable);
    }
  }
  void checkProcedureCall(shared_ptr<Procedure>& procedureTable,
                          shared_ptr<Node1> param) {
    shared_ptr<Node1> temp = param;
    vector<Variable> paramVec = procedureTable->signature;
    for (auto& param : paramVec) {
      if (!temp) {
        throw runtime_error("Too few arguments for procedure call");
      }
      temp->annotateType(localSymbolTable);
      if (param.type != temp->type) {
        throw runtime_error("Type mismatch for procedure call");
      }
      temp = temp->getChild("arglist", 2);
    }
    if (temp) {
      throw runtime_error("Too many arguments for procedure call");
    }
  }

  void checkStatment(shared_ptr<Node1> statements) {
    for (auto& child : statements->children) {
      if (child->lhs == "statement") {
        child->annotateType(localSymbolTable);

        if (child->getChild("lvalue", 0) && child->getChild("BECOMES", 1) &&
            child->getChild("expr", 2)) {
          auto lvalue = child->getChild("lvalue", 0);
          auto expr = child->getChild("expr", 2);
          if (lvalue->type != expr->type) {
            throw runtime_error("type error 1");
          }
        }
        if (child->getChild("PRINTLN", 0) && child->getChild("LPAREN", 1) &&
            child->getChild("expr", 2) && child->getChild("RPAREN", 3) &&
            child->getChild("SEMI", 4)) {
          auto expr = child->getChild("expr", 2);
          if (expr->type != "int") {
            throw runtime_error("type error 2");
          }
        }
        if (child->getChild("DELETE", 0) && child->getChild("LBRACK", 1) &&
            child->getChild("RBRACK", 2) && child->getChild("expr", 3) &&
            child->getChild("SEMI", 4)) {
          auto ex = child->getChild("expr", 3);
          if (ex->type != "int*") {
            throw runtime_error("type error 3");
          }
        }
        if (child->getChild("test", 2)) {
          auto test = child->getChild("test", 2);
          if (test->getChild("expr", 0)->type !=
              test->getChild("expr", 2)->type) {
            throw runtime_error("type error 4");
          }
        }

      } else if (child->lhs == "statements") {
        checkStatment(child);
      } else {
        throw runtime_error("not a statement");
      }
    }
  }
};

class ProcedureTable {
 public:
  unordered_map<string, shared_ptr<Procedure>> table;
  ProcedureTable(shared_ptr<Node1> procedures) {
    while (true) {
      if (procedures->getChild("main", 0)) {
        Add(make_shared<Procedure>(procedures->getChild("main", 0)));
        break;
      }
      auto procedureNode1 = procedures->getChild("procedure", 0);
      Add(make_shared<Procedure>(procedureNode1));
      procedures = procedures->getChild("procedures", 1);
    }
    while (procedures->getChild("procedure", 0) ||
           procedures->getChild("main", 0)) {
      if (procedures->getChild("procedure", 0)) {
        procedures = procedures->getChild("procedure", 0);
        Get(procedures->getChild("ID", 2)->lhs)->IDChecker(procedures, table);
      } else {
        procedures = procedures->getChild("main", 0);
        Get("wain")->IDChecker(procedures, table);
      }
    }
  }
  void Add(shared_ptr<Procedure> proc) {
    if (table.find(proc->name) != table.end()) {
      throw runtime_error("duplicate procedure declaration");
    }
    table[proc->name] = proc;
  }
  shared_ptr<Procedure> Get(string name) {
    if (table.find(name) != table.end()) {
      return table[name];
    } else {
      throw runtime_error("use of undeclared procedure");
    }
    return nullptr;
  }
};

class CFGObj {
  shared_ptr<Node1> head;
  unordered_map<string, bool> areTerminal;
  int labelCount = 0;
  bool isTerminal(string s) { return s.size() && isupper(s[0]); }

  shared_ptr<Node1> derive(istream& sin) {
    string s;
    getline(sin, s);
    stringstream iss{s};
    string left;
    iss >> left;
    auto parent = make_shared<Node1>(s);
    string oneOfRight;
    while (iss >> oneOfRight) {
      if (oneOfRight == ".EMPTY") {
        continue;
      }
      if (isTerminal(oneOfRight)) {
        string temp;
        getline(sin, temp);
        parent->addChild(make_shared<Node1>(temp));
      } else {
        parent->addChild(derive(sin));
      }
    }
    return parent;
  }

 public:
  CFGObj(istream& in) {
    head = derive(in);
    ProcedureTable table{head->getChild("procedures", 1)};
  }

  void print() {
    if (head) {
      head->printTree();
    } else {
      cout << "Empty tree\n";
    }
  }
  shared_ptr<Node1> getTree() { return head; }
};

void Node1::annotateType(VariableTable& localSymbolTable) {
  if (lhs == "NUM") {
    type = "int";
    return;
  }
  if (lhs == "ID") {
    auto temp = localSymbolTable.getVariable(rhs[0]);
    if (temp) {
      type = temp->type;
    } else {
      type = "int";
    }
    return;
  }
  if (lhs == "NULL") {
    type = "int*";
    return;
  }
  vector<string> types;
  for (auto& child : children) {
    child->annotateType(localSymbolTable);
    types.emplace_back(child->type);
  }
  if (lhs == "expr") {
    if (rhs.size() == 1) {
      type = types[0];
    } else if (rhs.size() == 3) {
      if (rhs[1] == "PLUS") {
        if (types[0] == "int" && types[2] == "int") {
          type = "int";
        } else if (types[0] == "int*" && types[2] == "int") {
          type = "int*";
        } else if (types[0] == "int" && types[2] == "int*") {
          type = "int*";
        } else {
          throw runtime_error("type error 11");
        }
      } else if (rhs[1] == "MINUS") {
        if (types[0] == "int" && types[2] == "int") {
          type = "int";
        } else if (types[0] == "int*" && types[2] == "int") {
          type = "int*";
        } else if (types[0] == "int*" && types[2] == "int*") {
          type = "int";
        } else {
          throw runtime_error("type error 22");
        }
      } else {
        throw runtime_error("expr error 1");
      }

    } else {
      throw runtime_error("expr error 2");
    }
  }
  if (lhs == "term") {
    if (rhs.size() == 1) {
      type = types[0];
    } else if (rhs.size() == 3) {
      if (types[0] == "int" && types[2] == "int") {
        type = "int";
      } else {
        throw runtime_error("type error");
      }
    } else {
      throw runtime_error("term error");
    }
  }
  if (lhs == "statement") {
    if (getChild("PRINTLN", 0)) {
      if (types[2] == "int") {
        type = "int";
      } else {
        throw runtime_error("type error");
      }
    }
    if (getChild("DELETE", 0)) {
      if (types[3] == "int*") {
      } else {
        throw runtime_error("why the f**k do we not have *int");
      }
    }
  }
  if (lhs == "factor") {
    if (getChild("ID", 0)) {
      if (rhs.size() == 1) {
        string na = getChild("ID", 0)->rhs[0];
        auto temp = localSymbolTable.getVariable(na);
        if (temp) {
          type = temp->type;
        } else {
          throw runtime_error("undefined variable");
        }
      }
      if (rhs.size() == 3) {
        string na = getChild("ID", 0)->rhs[0];
        auto temp = localSymbolTable.getVariable(na);
        if (temp) {
          throw runtime_error("Cannot call a variable");
        }
        type = "int";
      }
      if (rhs.size() == 4) {
        string na = getChild("ID", 0)->rhs[0];
        auto temp = localSymbolTable.getVariable(na);
        if (temp) {
          throw runtime_error("Cannot call a variable");
        }

        type = "int";
      }
    }
    if (getChild("NUM", 0)) {
      type = types[0];
    }
    if (getChild("NULL", 0)) {
      // wtf is this
      type = types[0];
    }
    if (getChild("LPAREN", 0) && getChild("expr", 1) && getChild("RPAREN", 2)) {
      type = types[1];
    }
    if (getChild("AMP", 0) && types[1] == "int") {
      type = "int*";
    }
    if (getChild("STAR", 0) && types[1] == "int*") {
      type = "int";
    }
    if (getChild("NEW", 0) && types[3] == "int") {
      type = "int*";
    }
  }
  if (lhs == "lvalue") {
    if (getChild("ID", 0)) {
      type = types[0];
    }
    if (getChild("STAR", 0)) {
      if (types[1] == "int*") {
        type = "int";
      } else {
        throw runtime_error("type error");
      }
    }
    if (getChild("LPAREN", 0)) {
      type = types[1];
    }
  }
  if (lhs == "arglist") {
    type = types[0];
  }
}
int debug = 0;

void push(int s) {
  cout << "sw $" << s << ", -4($30)" << endl;
  cout << "sub $30, $30, $4" << endl;
  debug += 1;
}

void pop(int s) {
  cout << "add $30, $30, $4" << endl;
  cout << "lw $" << s << ", -4($30)" << endl;
  debug -= 1;
}

void pop() {
  cout << "add $30, $30, $4" << endl;
  debug -= 1;
}

class ProcedureGen {
 public:
  int labelCount = 0;
  int paramOffset = 0;
  string name;
  unordered_map<string, int> offsetTable;
  ProcedureGen(shared_ptr<Node1> procedure) {
    name = (procedure->getChild("ID", 1) ? procedure->getChild("ID", 1)
                                         : procedure->getChild("WAIN", 1))
               ->rhs[0];
    Label("F" + name);
    int count = 0;
    if (procedure->lhs == "main") {
      paramOffset = 0;
      auto param1 = procedure->getChild("dcl", 3);
      auto param2 = procedure->getChild("dcl", 5);
      offsetTable[param1->getChild("ID", 1)->rhs[0]] = 8;
      offsetTable[param2->getChild("ID", 1)->rhs[0]] = 4;
      count += dclsGen(procedure->getChild("dcls", 8));

      statementsGen(procedure->getChild("statements", 9));
      exprGen(procedure->getChild("expr", 11));
    } else if (procedure->lhs == "procedure") {
      int countParam = 0;
      auto paramNode1 =
          procedure->getChild("params", 3)->getChild("paramlist", 0);
      while (paramNode1 && paramNode1->lhs == "paramlist") {
        countParam++;
        paramNode1 = paramNode1->getChild("paramlist", 2);
      }
      paramOffset = 4 * countParam;
      paramNode1 = procedure->getChild("params", 3)->getChild("paramlist", 0);
      while (paramNode1 && paramNode1->lhs == "paramlist") {
        auto temp = paramNode1->getChild("dcl", 0);
        offsetTable[temp->getChild("ID", 1)->rhs[0]] = paramOffset;
        paramOffset -= 4;
        paramNode1 = paramNode1->getChild("paramlist", 2);
      }
      count += dclsGen(procedure->getChild("dcls", 6));
      statementGen(procedure->getChild("statements", 7));
      exprGen(procedure->getChild("expr", 9));
    }
    jr(31);
  }

  int dclsGen(shared_ptr<Node1> dcls) {
    int countParam = 0;
    while (dcls->getChild("dcl", 1)) {
      countParam++;
      auto temp = dcls->getChild("dcl", 1);
      if (dcls->getChild("NUM", 3)) {
        offsetTable[temp->getChild("ID", 1)->rhs[0]] = paramOffset;
        cout << "; " << dcls->getChild("NUM", 3)->rhs[0] << " : " << paramOffset
             << endl;
        lis(3);
        word(dcls->getChild("NUM", 3)->rhs[0]);
        paramOffset -= 4;
      } else {
        offsetTable[temp->getChild("ID", 1)->rhs[0]] = paramOffset;
        lis(3);
        word(1);
        paramOffset -= 4;
      }
      push(3);
      dcls = dcls->getChild("dcls", 0);
    }
    return countParam;
  }
  void exprGen(shared_ptr<Node1> expr) {
    if (expr->getChild("term", 0)) {
      termGen(expr->getChild("term", 0));
    } else if (expr->getChild("expr", 0)) {
      if (expr->getChild("expr", 0)->type == "int" &&
          expr->getChild("term", 2)->type == "int") {
        termGen(expr->getChild("term", 2));
        push(3);
        exprGen(expr->getChild("expr", 0));
        pop(5);
        if (expr->getChild("PLUS", 1)) {
          add(3, 3, 5);
        } else if (expr->getChild("MINUS", 1)) {
          sub(3, 3, 5);
        } else {
          throw runtime_error("exprGen error1");
        }
      } else if (expr->getChild("expr", 0)->type == "int*" &&
                 expr->getChild("term", 2)->type == "int") {
        termGen(expr->getChild("term", 2));
        push(3);
        exprGen(expr->getChild("expr", 0));
        pop(5);
        if (expr->getChild("PLUS", 1)) {
          mult(5, 4);
          mflo(5);
          add(3, 3, 5);
        } else if (expr->getChild("MINUS", 1)) {
          mult(5, 4);
          mflo(5);
          sub(3, 3, 5);
        } else {
          throw runtime_error("exprGen error1");
        }

      } else if (expr->getChild("expr", 0)->type == "int" &&
                 expr->getChild("term", 2)->type == "int*") {
        exprGen(expr->getChild("expr", 0));
        push(3);
        termGen(expr->getChild("term", 2));
        pop(5);
        if (expr->getChild("PLUS", 1)) {
          mult(5, 4);
          mflo(5);
          add(3, 3, 5);
        }
      } else if (expr->getChild("expr", 0)->type == "int*" &&
                 expr->getChild("term", 2)->type == "int*") {
        termGen(expr->getChild("term", 2));
        push(3);
        exprGen(expr->getChild("expr", 0));
        pop(5);
        if (expr->getChild("MINUS", 1)) {
          sub(3, 3, 5);
          sdiv(3, 4);
          mflo(3);
        } else {
          throw runtime_error("exprGen error2");
        }
      }
    } else {
      throw runtime_error("exprGen error3");
    }
  }

  void termGen(shared_ptr<Node1> term) {
    if (term->rhs.size() == 1) {
      // 3A
      factorGen(term->getChild("factor", 0));
    } else if (term->rhs.size() == 3) {
      // 3A
      if (term->getChild("term", 0)->type == "int" &&
          term->getChild("factor", 2)->type == "int") {
        factorGen(term->getChild("factor", 2));
        push(3);
        termGen(term->getChild("term", 0));
        pop(5);
        if (term->getChild("STAR", 1)) {
          mult(3, 5);
          mflo(3);
        } else if (term->getChild("SLASH", 1)) {
          cout << "; SLASH" << endl;
          sdiv(3, 5);
          mflo(3);
        } else if (term->getChild("PCT", 1)) {
          sdiv(3, 5);
          mfhi(3);
        } else {
          throw runtime_error("termGen error");
        }
      }
    } else {
      throw runtime_error("termGen error");
    }
  }

  void factorGen(shared_ptr<Node1> factor) {
    if (factor->getChild("ID", 0)) {
      // 3A
      if (factor->rhs.size() == 1) {
        if (factor->getChild("ID", 0)) {
          string id = factor->getChild("ID", 0)->rhs[0];
          lw(3, offsetTable[id], 29);
        }
      } else if (factor->rhs.size() == 3) {
        // 5
        push(6);
        push(5);
        push(29);
        push(31);
        sub(29, 30, 4);
        string funcname = factor->getChild("ID", 0)->rhs[0];
        lis(5);
        word("F" + funcname);
        jalr(5);
        pop(31);
        pop(29);
        pop(5);
        pop(6);
      } else if (factor->rhs.size() == 4) {
        // 5
        push(6);
        push(5);
        push(29);
        push(31);
        int pushcount = 0;
        auto param = factor->getChild("arglist", 2);
        while (param) {
          pushcount++;
          exprGen(param->getChild("expr", 0));
          push(3);
          param = param->getChild("arglist", 2);
        }
        sub(29, 30, 4);
        string funcname = factor->getChild("ID", 0)->rhs[0];
        lis(5);
        word("F" + funcname);
        jalr(5);
        while (pushcount) {
          pop();
          pushcount--;
        }
        pop(31);
        pop(29);
        pop(5);
        pop(6);
      } else {
        throw runtime_error("factorGen error");
      }
    } else if (factor->getChild("NULL", 0)) {
      lis(3);
      word(1);
    } else if (factor->getChild("NUM", 0)) {
      lis(3);
      word(factor->getChild("NUM", 0)->rhs[0]);
    } else if (factor->getChild("LPAREN")) {
      exprGen(factor->getChild("expr", 1));
    } else if (factor->getChild("AMP")) {
      adrGen(factor->getChild("lvalue", 1));
    } else if (factor->getChild("STAR")) {
      factorGen(factor->getChild("factor", 1));
      lw(3, 0, 3);
    } else if (factor->getChild("NEW")) {
      exprGen(factor->children[3]);
      add(1, 3, 0);
      push(31);
      lis(5);
      word("new");
      jalr(5);
      pop(31);
      bne(3, 0, "1");
      add(3, 11, 0);
    } else {
      throw runtime_error("factorGen error");
    }
  }

  void statementsGen(shared_ptr<Node1> statements) {
    if (statements->getChild("statements", 0)) {
      statementsGen(statements->getChild("statements", 0));
    }
    if (statements->getChild("statement", 1)) {
      statementGen(statements->getChild("statement", 1));
    }
  }

  void statementGen(shared_ptr<Node1> statement) {
    if (statement->getChild("lvalue", 0)) {
      lvalueGen(statement->getChild("lvalue", 0));
      push(3);
      exprGen(statement->getChild("expr", 2));
      pop(5);
      sw(3, 0, 5);
    }
    if (statement->getChild("IF", 0)) {
      string label1 = "IFelse" + to_string(labelCount++) + name;
      string label2 = "IFend" + to_string(labelCount++) + name;
      testGen(statement->getChild("test", 2));
      beq(3, 0, label1);
      statementsGen(statement->getChild("statements", 5));
      beq(0, 0, label2);
      Label(label1);
      statementsGen(statement->getChild("statements", 9));
      Label(label2);
    }
    if (statement->getChild("WHILE", 0)) {
      string label1 = "Lloop" + to_string(labelCount++);
      string label2 = "Lend" + to_string(labelCount++);
      Label(label1);
      testGen(statement->getChild("test", 2));
      beq(3, 0, label2);
      statementsGen(statement->getChild("statements", 5));
      beq(0, 0, label1);
      Label(label2);
      
    }
    if (statement->getChild("PRINTLN", 0)) {
      push(1);
      exprGen(statement->getChild("expr", 2));
      add(1, 3, 0);
      push(31);
      jalr(23);
      pop(31);
      pop(1);
    }
    if (statement->getChild("DELETE", 0)) {
      delcount += 1;
      beq(3, 11, "isNULL" + to_string(delcount));
      push(31);
      push(1);
      exprGen(statement->getChild("expr", 3));
      add(1, 3, 0);
      lis(5);
      word("delete");
      jalr(5);
      pop(1);
      pop(31);
      Label("isNULL" + to_string(delcount));
    }
  }

  void testGen(shared_ptr<Node1> test) {
    if (test->rule == "test expr LT expr") {
      exprGen(test->getChild("expr", 0));
      push(3);
      exprGen(test->getChild("expr", 2));
      pop(5);
      string type = test->getChild("expr", 0)->type;  // int or int*

      if (type == "int") {
        cout << "slt $3, $5, $3" << endl;
      } else {  // type is int*. pointers are not negative. use unsigned
                // comparison
        cout << "sltu $3, $5, $3" << endl;
      }
    } else if (test->rule == "test expr EQ expr") {
      exprGen(test->getChild("expr", 0));
      push(3);
      exprGen(test->getChild("expr", 2));
      pop(5);

      string type = test->getChild("expr", 0)->type;  // int or int*
                                                      // int or int*

      if (type == "int") {
        cout << "slt $6, $5, $3" << endl;
        cout << "slt $7, $3, $5" << endl;
      } else {  // type is int*. pointers are not negative. use unsigned
                // comparison
        cout << "sltu $6, $5, $3" << endl;
        cout << "sltu $7, $3, $5" << endl;
      }
      cout << "add $3, $6, $7" << endl;
      cout << "sub $3, $11, $3" << endl;
    } else if (test->rule == "test expr NE expr") {
      exprGen(test->getChild("expr", 0));
      push(3);
      exprGen(test->getChild("expr", 2));
      pop(5);

      string type = test->getChild("expr", 0)->type;

      if (type == "int") {
        cout << "slt $6, $5, $3" << endl;
        cout << "slt $7, $3, $5" << endl;
      } else {
        cout << "sltu $6, $5, $3" << endl;
        cout << "sltu $7, $3, $5" << endl;
      }
      cout << "add $3, $6, $7" << endl;
    } else if (test->rule == "test expr LE expr") {
      exprGen(test->getChild("expr", 2));
      push(3);
      exprGen(test->getChild("expr", 0));
      pop(5);

      string type = test->getChild("expr", 0)->type;  // int or int*
                                                      // int or int*

      if (type == "int") {
        cout << "slt $3, $5, $3" << endl;
      } else {
        cout << "sltu $3, $5, $3" << endl;
      }
      cout << "sub $3, $11, $3" << endl;  // take the logical inverse
    } else if (test->rule == "test expr GE expr") {
      exprGen(test->getChild("expr", 0));
      push(3);
      exprGen(test->getChild("expr", 2));
      pop(5);
      string type = test->getChild("expr", 0)->type;  // int or int*
                                                      // int or int*

      if (type == "int") {
        cout << "slt $3, $5, $3" << endl;
      } else {
        cout << "sltu $3, $5, $3" << endl;
      }
      cout << "sub $3, $11, $3" << endl;  // take the logical inverse
    } else {                              // test expr GT expr
      exprGen(test->getChild("expr", 2));
      push(3);
      exprGen(test->getChild("expr", 0));
      pop(5);
      string type = test->getChild("expr", 0)->type;  // int or int*
                                                      // int or int*

      if (type == "int") {
        cout << "slt $3, $5, $3" << endl;
      } else {
        cout << "sltu $3, $5, $3" << endl;
      }
    }
  }

  void lvalueGen(shared_ptr<Node1> lvalue) {
    if (lvalue->getChild("ID", 0)) {
      string id = lvalue->getChild("ID", 0)->rhs[0];
      lis(3);
      word(offsetTable[id]);
      add(3, 3, 29);
    } else if (lvalue->getChild("STAR", 0)) {
      factorGen(lvalue->getChild("factor", 1));
    } else if (lvalue->getChild("LPAREN", 0)) {
      lvalueGen(lvalue->getChild("lvalue", 1));
    } else {
      throw runtime_error("lvalueGen error");
    }
  }

  void adrGen(shared_ptr<Node1> tree) {
    if (tree->rule == "lvalue ID") {
      string id = tree->getChild("ID", 0)->rhs[0];
      lis(3);
      word(offsetTable[id]);
      cout << "add $3, $3, $29" << endl;
      return;
    } else if (tree->rule == "lvalue STAR factor") {
      factorGen(tree->getChild("factor", 1));
    } else if (tree->rule == "lvalue LPAREN lvalue RPAREN") {
      adrGen(tree->getChild("lvalue", 1));
    } else {
      throw runtime_error("adrGen error");
    }
  }
};

class CodeGenerator {
 public:
  CodeGenerator(shared_ptr<Node1> tree) {
    cout << ".import print" << endl;
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "lis $12" << endl;
    cout << ".word init" << endl;
    cout << "lis $23" << endl;
    cout << ".word print" << endl;

    push(1);
    push(2);

    auto proceduresTemp = tree->getChild("procedures", 1);
    while (true) {
      if (proceduresTemp->getChild("main", 0)) {
        if (proceduresTemp->getChild("main", 0)
                    ->getChild("dcl", 3)
                    ->getChild("type", 0)
                    ->rhs[0] == "INT" &&
            proceduresTemp->getChild("main", 0)
                    ->getChild("dcl", 3)
                    ->getChild("type", 0)
                    ->rhs.size() == 1) {
          add(2, 0, 0);
          push(31);
          jalr(12);
          pop(31);
        }
        break;
      }
      proceduresTemp = proceduresTemp->getChild("procedures", 1);
    }
    sub(29, 30, 4);
    beq(0, 0, "Fwain");
    auto procedures = tree->getChild("procedures", 1);
    while (true) {
      if (procedures->getChild("main", 0)) {
        ProcedureGen(procedures->getChild("main", 0));
        break;
      }
      ProcedureGen(procedures->getChild("procedure", 0));
      procedures = procedures->getChild("procedures", 1);
    }
    pop();
    pop();
  }
};

int main() {
  try {
    ifstream s("dfa.dfa");
    stringstream asms;
    string temp;
    while (getline(cin, temp)) {
      asms << temp << "\n";
    };
    DFA dfa(s);
    deque<Token> tokens = tokenHelper(dfa, asms);
    wlp4parse parser{tokens};
    parser.parse();

    CFGObj cfg{out};
    CodeGenerator{cfg.getTree()};
    // cout << debug << endl;
  } catch (runtime_error& e) {
    cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
}
