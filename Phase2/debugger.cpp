/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for multiple breakpoints and step-by-step
// execution of straight-line code. Uses nuPython interpreter as 
// execution engine.
//
// Authors:
//    Ata Guvendi
//    Prof. Joe Hummel
//    Prof. Yiji Zhang
// 
// Modified by Jad Dibs
//
// Northwestern University
// CS 211
//

#include <iostream>
#include <cassert>
#include <utility>
#include <string>

#include "debugger.h"
#include "execute.h"
#include "programgraph.h"

using namespace std;

//
// getWhileLoopBodyLines
//
// Obtains all line numbers in while loop body (loopBody) of while loop statement (whileStmt) 
// and maps these lines to false in class field breakpoints,
// later used to identify breakpoint lines by setting such lines to true
//
void Debugger::getWhileLoopBodyLines(struct STMT* loopBody, struct STMT* whileStmt)
{
  struct STMT* cur = loopBody; // keeps track of current statement in program

  //
  // iterates throughout loopBody to identify all valid lines
  //
  while (cur != whileStmt) 
  {

    this->breakpoints[cur->line] = false; // initializes key (line number) and its value (false) in map breakpoints

    //
    // updates cur pointer to the next statement in loop body
    //
    if (cur->stmt_type == STMT_ASSIGNMENT) 
    {
      cur = cur->types.assignment->next_stmt;
    }
    else if (cur->stmt_type == STMT_FUNCTION_CALL) 
    {
      cur = cur->types.function_call->next_stmt;
    }
    else if (cur->stmt_type == STMT_PASS) 
    {
      cur = cur->types.pass->next_stmt;
    }
    else if (cur->stmt_type == STMT_WHILE_LOOP) // nested while loop
    { 
      //
      // calls method recursively for nested while loop body
      //
      struct STMT* loopBody = cur->types.while_loop->loop_body;
      this->getWhileLoopBodyLines(loopBody, cur);

      cur = cur->types.while_loop->next_stmt; // advances to next statement after nested while loop
    }
    else 
    {
      //
      // nothing should happen
      //
    }
  }
}


//
// getProgramLines
//
// Obtains all line numbers in Program and maps these lines to false in class field breakpoints,
// later used to identify breakpoint lines by setting such lines to true
//
void Debugger::getProgramLines()
{
  struct STMT* cur = this->Program; // keeps track of current statement in program

  //
  // iterates throughout Program to identify all valid lines
  //
  while (cur != nullptr) 
  {

    this->breakpoints[cur->line] = false; // initializes key (line number) and its value (false) in map breakpoints

    //
    // updates cur pointer to the next statement in Program (special case for while loops)
    //
    if (cur->stmt_type == STMT_ASSIGNMENT)
    {
      cur = cur->types.assignment->next_stmt;
    }
    else if (cur->stmt_type == STMT_FUNCTION_CALL) 
    {
      cur = cur->types.function_call->next_stmt;
    }
    else if (cur->stmt_type == STMT_PASS) 
    {
      cur = cur->types.pass->next_stmt;
    }
    else if (cur->stmt_type == STMT_WHILE_LOOP) 
    {
      struct STMT* loopBody = cur->types.while_loop->loop_body;
      this->getWhileLoopBodyLines(loopBody, cur);
      cur = cur->types.while_loop->next_stmt;
    }
    else 
    {
      //
      // nothing should happen
      //
    }
  }
}


//
// keyInBreakpoints
//
// Checks if breakpoint line bpLine is a key present in map breakpoints;
// Returns true if present, false if not present
//
bool Debugger::keyInBreakpoints(int bpLine)
{
  auto iter = this->breakpoints.find(bpLine); // check if input breakpoint exists in map breakpoints
  if (iter != this->breakpoints.end()) return true; // line breakpoint exists
  else return false; // line breakpoint does not exist
}


//
// printValue
//
// Prints the contents of a RAM cell, both type and value.
//
void Debugger::printValue(string varname, struct RAM_VALUE* value)
{
  cout << varname << " ("; 
  
  switch (value->value_type) 
  {
    
    case RAM_TYPE_INT:
      cout << "int): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_REAL:
      cout << "real): " << value->types.d << endl;
      break;
      
    case RAM_TYPE_STR:
      cout << "str): " << value->types.s << endl;
      break;
      
    case RAM_TYPE_PTR:
      cout << "ptr): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_BOOLEAN:
      cout << "bool): " << value->types.i << endl;
      break;
    
    case RAM_TYPE_NONE:
      cout << "none): " << "None" << endl;
      break;
  }//switch
}


//
// breakLink
//
// Breaks the link between the cur statement and the one 
// that follows; returns a pointer to the statement that
// follows before the link was broken.
//
struct STMT* Debugger::breakLink(struct STMT* cur)
{
  STMT* next = nullptr;
  
  if (cur == nullptr) // nothing to break:
    return nullptr;
  
  if (cur->stmt_type == STMT_ASSIGNMENT) 
  {
    next = cur->types.assignment->next_stmt;
    cur->types.assignment->next_stmt = nullptr;
  }
  else if (cur->stmt_type == STMT_FUNCTION_CALL) 
  {
    next = cur->types.function_call->next_stmt;
    cur->types.function_call->next_stmt = nullptr;
  }
  else if (cur->stmt_type == STMT_PASS) 
  {
    next = cur->types.pass->next_stmt;
    cur->types.pass->next_stmt = nullptr;
  }
  else if (cur->stmt_type == STMT_WHILE_LOOP) 
  {
    // solved in breakLinksWhile
  }
  else // unexpected stmt type:
  { 
    cout << "**ERROR" << endl;
    cout << "**ERROR: unexpected stmt type in breakLink" << endl;
    cout << "**ERROR" << endl;
    assert(false);
  }
  
  return next;
}


//
// breakLinksWhile
//
// For a while loop statement, isolates the cur statement and breaks both links (loop_body and next_stmt); 
// returns a pair of pointers to the loop_body and next_stmt statements.
//
pair<struct STMT*, struct STMT*> Debugger::breakLinksWhile(struct STMT* cur)
{
  struct STMT* loopBody = cur->types.while_loop->loop_body;
  struct STMT* nextStmt = cur->types.while_loop->next_stmt;
  cur->types.while_loop->loop_body = nullptr;
  cur->types.while_loop->next_stmt = nullptr;

  return make_pair(loopBody, nextStmt);
}


//
// restoreLink
//
// Restores the link between the cur statement and the one 
// that follows.
//
void Debugger::restoreLink(struct STMT* cur, struct STMT* next)
{
  if (cur == nullptr) // nothing to restore:
    return;
  
  if (cur->stmt_type == STMT_ASSIGNMENT) 
  {
    cur->types.assignment->next_stmt = next;
  }
  else if (cur->stmt_type == STMT_FUNCTION_CALL) 
  {
    cur->types.function_call->next_stmt = next;
  }
  else if (cur->stmt_type == STMT_PASS) 
  {
    cur->types.pass->next_stmt = next;
  }
  else if (cur->stmt_type == STMT_WHILE_LOOP) 
  {
    // solved in restoreLinksWhile
  }
  else // unexpected stmt type:
  { 
    cout << "**ERROR" << endl;
    cout << "**ERROR: unexpected stmt type in restoreLink" << endl;
    cout << "**ERROR" << endl;
    assert(false);
  }
  
  return;
}


//
// restoreLinkWhile
//
// For a while statement, restores the links between the cur statement and the two 
// statements that follow (loop_body and next_stmt).
//
void Debugger::restoreLinksWhile(struct STMT* cur, pair<struct STMT*, struct STMT*> nextStmts)
{
  cur->types.while_loop->loop_body = nextStmts.first;
  cur->types.while_loop->next_stmt = nextStmts.second;
}


//
// hCommand
//
// Outputs the help commands
//
void Debugger::hCommand()
{
  cout << "Available commands:"
  << endl << "r -> Run the program / continue from a breakpoint"
  << endl << "s -> Step to next stmt by executing current stmt"
  << endl << "b n -> Breakpoint at line n"
  << endl << "rb n -> Remove breakpoint at line n"
  << endl << "lb -> List all breakpoints"
  << endl << "cb -> Clear all breakpoints"
  << endl << "p varname -> Print variable"
  << endl << "sm -> Show memory contents"
  << endl << "ss -> Show state of debugger"
  << endl << "w -> What line are we on?"
  << endl << "q -> Quit the debugger"
  << endl;
}


//
// rAndsCommands
//
// s command runs one line of Program, r command runs the entire Program,
// with breakpoints the behavior is slightly different
//
// Note: Some parameters made pass by reference to adjust variables in Debugger::run accordingly
//
void Debugger::rAndsCommands(string cmd, struct STMT*& curStmt, struct STMT*& nextStmt, pair<struct STMT*, struct STMT*>& nextStmts)
{   
  //
  // execute current stmt via nuPython interpreter:
  //
  while (curStmt != nullptr) 
  {
    if (keyInBreakpoints(curStmt->line) && this->breakpoints[curStmt->line]) // curStmt is a breakpoint
    { 
      if (!this->HitBP) // breakpoint was not hit in previous command
      { 
        this->HitBP = true;
        cout << "hit breakpoint at line " << curStmt->line << endl;
        
        break; // stops execution for both r and s commands
      }
    }

    //
    // curStmt not breakpoint or breakpoint was hit in previous command
    //
    this->HitBP = false;

    if (curStmt->stmt_type == STMT_WHILE_LOOP) // calling execute_expr() on while loop statement
    { 
      //
      // evaluate while loop condition
      //
      struct RAM_VALUE* rv = execute_expr(curStmt, this->Memory, curStmt->types.while_loop->condition);

      //
      // what happened during execution?
      //
      if (rv == NULL) // there was an error, we've complete execution:
      { 
        this->State = "Completed";
        
        // we need to repair the program graph:
        restoreLinksWhile(curStmt, nextStmts);
        
        break;
      }

      //
      // advance one stmt:
      //
      restoreLinksWhile(curStmt, nextStmts);
      
      if (rv->types.i == 0) // skips loop body if while loop condition is false
      { 
        curStmt = curStmt->types.while_loop->next_stmt;
      }
      else // goes to loop body if while loop condition is true
      { 
        curStmt = curStmt->types.while_loop->loop_body;
      }

      ram_free_value(rv);
    }
    else // calling execute() on other statements
    { 
      // 
      // execute this line:
      //
      struct ExecuteResult er = execute(curStmt, this->Memory);

      //
      // what happened during execution?
      //
      if (!er.Success) // there was an error, we've complete execution:
      { 
        this->State = "Completed";
        
        // we need to repair the program graph:
        restoreLink(curStmt, nextStmt);
        
        break;
      }

      //
      // advance one stmt:
      //
      restoreLink(curStmt, nextStmt);

      curStmt = nextStmt;
    }

    //
    // update nextStmts or nextStmt depending on curStmt type
    //
    if (curStmt != nullptr) 
    {
      if (curStmt->stmt_type == STMT_WHILE_LOOP) 
      {
        nextStmts = breakLinksWhile(curStmt);
      }
      else 
      {
        nextStmt = breakLink(curStmt);
      }
    }

    // 
    // are we stepping? if so, exit loop:
    //
    if (cmd == "s")
      break;
  } // while

  //
  // loop has ended, why? There are 3 cases:
  //   1. hit a breakpoint or we stepped once
  //   2. semantic error
  //   3. ran to completion
  //
  if (curStmt == nullptr) // we ran to completion:
  {  
    this->State = "Completed";
    // program graph is fine since we ran to the end
  }
  else if (this->State == "Completed") // semantic error
  {  
    //
    // handled inside loop
    //
  }
  else 
  {
    //
    // else we did step, nothing to do here
    //
  }
}


//
// bCommand
//
// Sets a breakpoint at input line number
//
void Debugger::bCommand()
{
  int breakpoint;
  cin >> breakpoint; // user input

  if (this->keyInBreakpoints(breakpoint)) // line breakpoint exists in map breakpoints
  { 
    if (this->breakpoints[breakpoint] == false) 
    {
      this->breakpoints[breakpoint] = true;
      cout << "breakpoint set" << endl;
    }
    else 
    {
      cout << "breakpoint already exists" << endl;
    }
  }
  else // line breakpoint does not exist
  { 
    cout << "no such line" << endl;
  }
}


//
// rbCommand
//
// Removes breakpoint at input line number
//
void Debugger::rbCommand()
{
  int bpToRemove;
  cin >> bpToRemove; // user input

  if (this->keyInBreakpoints(bpToRemove)) // line bpToRemove exists in map breakpoints
  { 
    if (this->breakpoints[bpToRemove] == true) // line bpToRemove is a breakpoint, so remove
    { 
      this->breakpoints[bpToRemove] = false;
      cout << "breakpoint removed" << endl;
    }
    else // line bpToRemove is not a breakpoint
      cout << "no such breakpoint" << endl;
  }
  else // line bpToRemove does not exist in map breakpoints, so not a breakpoint
  { 
    cout << "no such breakpoint" << endl;
  }
}


//
// lbCommand
//
// Outputs all breakpoints in map breakpoints (keys with values of true)
//
void Debugger::lbCommand()
{
  bool anyBPs = false; // tracks if there are any breakpoints in map breakpoints
  string outputBPs = "breakpoints on lines:";

  for (const auto &line : this->breakpoints) // iterates through map breakpoints to identify breakpoints
  {
    if (line.second) 
    {
      string lineN = to_string(line.first);
      outputBPs += " " + lineN;
      anyBPs = true;
    }
  }

  if (!anyBPs) // no breakpoints
    cout << "no breakpoints";
  else
    cout << outputBPs;
  
  cout << endl;
}


//
// cbCommand
//
// Clears (deletes) all breakpoints in map breakpoints
//
void Debugger::cbCommand()
{
  for (auto &line : this->breakpoints) // iterates through map breakpoints to identify breakpoints
  {
    if (line.second) // breakpoint exists, so remove
    { 
      line.second = false;
    }
  }

  cout << "breakpoints cleared" << endl;
}


//
// pCommand
//
// Outputs the type and value of an input variable
//
void Debugger::pCommand()
{
  string varname;
  cin >> varname;
  
  const char* name = varname.c_str();
  
  struct RAM_VALUE* value = ram_read_cell_by_name(this->Memory, (char*) name);
  
  if (value == nullptr) 
  {
    cout << "no such variable" << endl;
  }
  else 
  {
    printValue(varname, value);
    ram_free_value(value);
  }
}


//
// wCommand
//
// Outputs the line of the current statement in the program
//
void Debugger::wCommand(struct STMT* cur)
{
  if (this->State == "Loaded") 
  {
    cout << "line " << cur->line << endl;
    // programgraph_print(curStmt);
  }
  else if (this->State == "Completed") 
  {
    cout << "completed execution" << endl;
  }
  else // we are running:
  { 
    cout << "line " << cur->line << endl;
    // programgraph_print(curStmt);
  }
}


//
// constructor:
//
Debugger::Debugger(struct STMT* program)
  : State("Loaded"), Program(program), Memory(nullptr)
{
  this->Memory = ram_init();
}


//
// destructor:
//
Debugger::~Debugger()
{
  ram_destroy(this->Memory);
}


//
// run:
//
// Run the debugger for one execution run of the input program.
//
void Debugger::run()
{
  string cmd;

  //
  // find all line numbers in Program
  //
  this->getProgramLines();
  
  //
  // controls where we start execution from:
  //
  struct STMT* curStmt = this->Program;
  
  //
  // we're going to execute stmt-by-stmt, so we have to break
  // the program graph so that if we run the program, we just
  // run the first stmt:
  //
  pair<struct STMT*, struct STMT*> nextStmts;
  struct STMT* nextStmt;

  if (curStmt->stmt_type == STMT_WHILE_LOOP) // for while loop statements
  { 
    nextStmts = breakLinksWhile(curStmt);
  }
  else // for other statements
  { 
    nextStmt = breakLink(curStmt);
  }

  //
  // command loop until quit is entered:
  //
  while (true) 
  {
    
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > " << endl;
    cin >> cmd;
    
    if (cmd == "q") 
    {
      break;
    }
    else if (cmd == "h") 
    {
      
      this->hCommand();
    }
    else if (cmd == "r" || cmd == "s") 
    {
      
      //
      // run, or continue running, the program:
      //
      if (this->State == "Completed") 
      {
        cout << "program has completed" << endl;
        continue; // skip the code below and repeat the loop for next cmd:
      }
      
      if (this->State == "Loaded")
        this->State = "Running";
      
      rAndsCommands(cmd, curStmt, nextStmt, nextStmts);
    }
    else if (cmd == "b") 
    {
      
      this->bCommand();
    }
    else if (cmd == "rb") 
    {

      this->rbCommand();
    }
    else if (cmd == "lb") 
    {

      this->lbCommand();
    }
    else if (cmd == "cb") 
    {

      this->cbCommand();
    }
    else if (cmd == "p")
    {
      
      this->pCommand();
    }
    else if (cmd == "sm") 
    {
      
      ram_print(this->Memory);
    }
    else if (cmd == "ss") 
    {
      
      cout << this->State << endl;
    }
    else if (cmd == "w") 
    {
      
      this->wCommand(curStmt);
    }
    else 
    {
      
      cout << "unknown command" << endl;
    }
    
  }//while
  
  //
  // at this point execution has completed (or the user quit
  // early). Repair the program graph if need be:
  //
  if (curStmt != nullptr) 
  {
    if (curStmt->stmt_type == STMT_WHILE_LOOP) 
    {
      restoreLinksWhile(curStmt, nextStmts);
    }
    else 
    {
      restoreLink(curStmt, nextStmt);
    }
  }
  
}//run
