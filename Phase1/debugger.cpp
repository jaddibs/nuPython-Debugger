/*debugger.cpp*/

//
// Debugger for nuPython, C++ edition! Provides a simple gdb-like
// interface, with support for multiple breakpoints and step-by-step
// execution of straight-line code. Uses nuPython interpreter as 
// execution engine.
//
// Modified by Jad Dibs
// Northwestern University
// CS 211
//

#include <iostream>

#include "debugger.h"
#include "execute.h"

using namespace std;

//
// Constructor
//
// Constructs Debugger object
//
Debugger::Debugger(struct STMT* program)
  : Program(program)
{}

//
// Destructor
//
// Destructs Debugger object
//
Debugger::~Debugger()
{
  ram_destroy(Memory);
}

//
// run
//
// Runs the debugger on Program, stops when user inputs q
//
void Debugger::run()
{
  string cmd;

  struct STMT* rest = this->Program; // keep track of Program from next statement to execute to end
  
  //
  // command loop until quit is entered:
  //
  while (true) {
    
    cout << endl;
    cout << "Enter a command, type h for help. Type r to run. > " << endl;
    cin >> cmd;
    
    if (cmd == "q") // input q (quit debugger)
    {
      return;
    }
    else if (cmd == "h") // input h (help with available command definitions)
    {
      this->input_h();
    }
    else if (cmd == "r") // input r (run input file from next statement to end, or first statement to end if from the beginning)
    {
      if (this->State == "Completed")
      {
        cout << "program has completed" << endl;
        continue;
      }
      execute(rest, Memory);
      this->State = "Completed";
    }
    else if (cmd == "s") // input s (step to next statement by executing current statement)
    {
      struct STMT* stmt = rest; // current statement in Program to execute
      
      //
      // update current state of program
      //
      if (this->State == "Completed")
      {
        cout << "program has completed" << endl;
        continue;
      }
      else if (this->State == "Loaded")
      {
        this->State = "Running";
      }

      //
      // execute line of current statement in Program and update to next statement
      //
      if (stmt->stmt_type == STMT_ASSIGNMENT)
      {
        struct STMT* next = stmt->types.assignment->next_stmt; // temp variable for next_stmt of stmt
        stmt->types.assignment->next_stmt = nullptr; // isolates current statement in Program
        struct ExecuteResult res = execute(stmt, Memory);

        stmt->types.assignment->next_stmt = next; // restore next_stmt

        if (!res.Success) // if execute is not successful (due to semantic error in Program)
        {
          this->State = "Completed";
          continue;
        }

        rest = rest->types.assignment->next_stmt;
      }
      else if (stmt->stmt_type == STMT_FUNCTION_CALL)
      {
        struct STMT* next = stmt->types.function_call->next_stmt; // temp variable for next_stmt of stmt
        stmt->types.function_call->next_stmt = nullptr; // isolates current statement in Program
        struct ExecuteResult res = execute(stmt, Memory);

        stmt->types.function_call->next_stmt = next; // restore next_stmt

        if (!res.Success) // if execute is not successful (due to semantic error in Program)
        {
          this->State = "Completed";
          continue;
        }

        rest = rest->types.function_call->next_stmt;
      }
      else if (stmt->stmt_type == STMT_PASS)
      {
        struct STMT* next = stmt->types.pass->next_stmt; // temp variable for next_stmt of stmt
        stmt->types.pass->next_stmt = nullptr; // isolates current statement in Program
        struct ExecuteResult res = execute(stmt, Memory);

        stmt->types.pass->next_stmt = next; // restore next_stmt

        if (!res.Success) // if execute is not successful (due to semantic error in Program)
        {
          this->State = "Completed";
          continue;
        }

        rest = rest->types.pass->next_stmt; // update rest to point to next statement

        if (rest == nullptr)
        {
          this->State = "Completed";
        }
      }
    }
    else if (cmd == "w") // input w (output line number that debugger will execute next)
    {
      if (this->State == "Completed")
      {
        cout << "completed execution" << endl;
        continue;
      }
      int next_line = rest->line;
      cout << "line " << next_line << endl;
    }
    else if (cmd == "ss") // input ss (output state of debugger State, either Loaded, Running, or Completed)
    {
      cout << this->State << endl;
    }
    else if (cmd == "sm") // input sm (output contents of the memory)
    {
      ram_print(Memory);
    }
    else if (cmd == "p") // input p (outputs type and value of input variable)
    {
      string var_name;
      cin >> var_name; // input desired variable

      const char* name = var_name.c_str();
      char* var = const_cast<char*>(name);

      struct RAM_VALUE* value = ram_read_cell_by_name(Memory, var);

      //
      // print the type and value of variable for the input variable in the format VARNAME (TYPE): VARVALUE
      //
      if (value == nullptr)
      {
        cout << "no such variable" << endl;
      }
      else if (value->value_type == RAM_TYPE_INT)
      {
        cout << var_name << " (" << "int" << ")" << ": " << value->types.i << endl;
      }
      else if (value->value_type == RAM_TYPE_REAL)
      {
        cout << var_name << " (" << "real" << ")" << ": " << value->types.d << endl;
      }
      else if (value->value_type == RAM_TYPE_STR)
      {
        cout << var_name << " (" << "str" << ")" << ": " << value->types.s << endl;
      }
      else if (value->value_type == RAM_TYPE_PTR)
      {
        cout << var_name << " (" << "ptr" << ")" << ": " << value->types.i << endl;
      }
      else if (value->value_type == RAM_TYPE_BOOLEAN)
      {
          cout << var_name << " (" << "bool" << ")" << ": " << value->types.i << endl;
      }
      else
      {
        cout << var_name << " (" << "none" << ")" << ": " << endl;
      }

      ram_free_value(value);
    }
    else // another other input
    {
      cout << "unknown command" << endl;
    }
    
  }//while

  programgraph_destroy(rest);
}//run

//
// input_h
//
// Outputs the help statements if h is input by the user
//
void Debugger::input_h()
{
  cout << "Available commands:"
  << endl << "r -> Run the program / continue from a breakpoint"
  << endl << "s -> Step to next stmt by executing current stmt"
  << endl << "p varname -> Print variable"
  << endl << "sm -> Show memory contents"
  << endl << "ss -> Show state of debugger"
  << endl << "w -> What line are we on?"
  << endl << "q -> Quit the debugger"
  << endl;
}