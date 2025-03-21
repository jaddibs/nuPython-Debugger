/*debugger.h*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for one breakpoint in straight-line code.
// Uses nuPython interpreter as execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Hummel
// 
// Modified by Jad Dibs
//
// Northwestern University
// CS 211
//

#pragma once

#include <string>
#include <map>

#include "programgraph.h"
#include "ram.h"

using namespace std;

class Debugger
{
private:
  string State;
  map<int, bool> breakpoints;
  bool HitBP; // keeps track of if breakpoint was already hit (used in Debugger::rAndsCommands)
  struct STMT* Program;
  struct RAM*  Memory;
  
  void getWhileLoopBodyLines(struct STMT* loopBody, struct STMT* whileStmt);
  void getProgramLines();
  bool keyInBreakpoints(int bpLine);
  void printValue(string varname, struct RAM_VALUE* value);
  struct STMT* findStmt(struct STMT* cur, int lineNum);
  struct STMT* breakLink(struct STMT* cur);
  pair<struct STMT*, struct STMT*> breakLinksWhile(struct STMT* cur);
  void restoreLink(struct STMT* cur, struct STMT* next);
  void restoreLinksWhile(struct STMT* cur, pair<struct STMT*, struct STMT*> nextStmts);
  void hCommand();
  void rAndsCommands(string cmd, struct STMT*& curStmt, struct STMT*& nextStmt, pair<struct STMT*, struct STMT*>& nextStmts);
  void bCommand();
  void rbCommand();
  void lbCommand();
  void cbCommand();
  void pCommand();
  void wCommand(struct STMT* cur);

public:
  Debugger(struct STMT* program);

  ~Debugger();

  void run();
};

