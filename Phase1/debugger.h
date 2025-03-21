/*debugger.h*/

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

#pragma once

#include "programgraph.h"
#include "ram.h"

using namespace std;

class Debugger
{
  struct STMT* Program;
  struct RAM* Memory = ram_init();
  string State = "Loaded";

public:
  //
  // Constructor
  //
  // Constructs Debugger object
  //
  Debugger(struct STMT* program);

  //
  // Destructor
  //
  // Destructs Debugger object
  //
  ~Debugger();

  //
  // run
  //
  // Runs the debugger on Program, stops when user inputs q
  //
  void run();

  //
  // input_h
  //
  // Outputs the help statements if h is input by the user
  //
  void input_h();

};

