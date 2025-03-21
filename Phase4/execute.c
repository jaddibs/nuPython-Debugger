/*execute.c*/

//
// Executes nuPython program, given as a Program Graph.
//
// Jad Dibs
// Northwestern University
// CS211
// Winter Quarter, 2025
// 
// Starter code: Prof. Joe Hummel, Prof. Yiji Zhang
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"


//
// Private functions:
//

//
// execute_assignment
//
// Given a nuPython program graph assignment statement and a memory, executes assignment
// Returns true if statement executes successfully, and false if not
//
static bool execute_assignment(struct STMT* stmt, struct RAM* memory);

//
// execute_binary_expression
//
// Calculates result of a binary expression (like "y + 10"), only handles integers
// Returns true if function is successful, and false if not
// Updates the integer result in a parameter int* result
//
static bool execute_binary_expression(struct EXPR* expr, int* result, struct RAM* memory, int line);

//
// retrieve_value
//
// Given a UNARY_EXPR* parameter, which represents a term in an expression,
// Returns a non-NULL char* of the variable name if the term is an undefined variable
// Returns NULL if the term is a defined variable or an integer literal
// Updates the integer value in a parameter int* value
//
static char* retrieve_value(struct UNARY_EXPR* term, int* value, struct RAM* memory);

//
// execute_function_call
//
// Given a nuPython program graph function call statement and a memory, executes function call
// Returns true if statement executes successfully, and false if not
//
static bool execute_function_call(struct STMT* stmt, struct RAM* memory);

//
// Public functions:
//

//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory)
{
  struct STMT* stmt = program;

  // traverse through the program statements:
  while (stmt != NULL)
  {
    if (stmt->stmt_type == STMT_ASSIGNMENT)
    {
      bool success = execute_assignment(stmt, memory);

      if (!success)
        return;

      stmt = stmt->types.assignment->next_stmt;
    }
    else if (stmt->stmt_type == STMT_FUNCTION_CALL)
    {
      bool success = execute_function_call(stmt, memory);
      
      if (!success)
        return;

      stmt = stmt->types.function_call->next_stmt;
    }
    else
    {
      assert(stmt->stmt_type == STMT_PASS);
  
      stmt = stmt->types.pass->next_stmt;
    }
  }
}

//
// Private functions implementations:
//

static bool execute_assignment(struct STMT* stmt, struct RAM* memory)
{
  if (stmt->stmt_type != STMT_ASSIGNMENT)
    return false;

  char* var_name = stmt->types.assignment->var_name;
  struct VALUE* rhs = stmt->types.assignment->rhs;

  if (rhs->value_type != VALUE_EXPR)
    return false;

  struct EXPR* expr = rhs->types.expr;

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT; // assuming all variables are integers

  if (!expr->isBinaryExpr)
  {
    int expr_value;

    char* undef_var = retrieve_value(expr->lhs, &expr_value, memory);

    if (undef_var != NULL) // undefined variable
    {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", undef_var, stmt->line);
      return false;
    }

    value.types.i = expr_value;
  }
  else
  {
    int result;

    bool success = execute_binary_expression(expr, &result, memory, stmt->line);

    if (!success)
      return false;

    value.types.i = result;
  }

  ram_write_cell_by_name(memory, value, var_name);

  return true;
}

static bool execute_binary_expression(struct EXPR* expr, int* result, struct RAM* memory, int line)
{
  int val_lhs;
  int val_rhs;

  char* lhs_unary = retrieve_value(expr->lhs, &val_lhs, memory);
  char* rhs_unary = retrieve_value(expr->rhs, &val_rhs, memory);

  if (lhs_unary != NULL)
  {
    printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", lhs_unary, line);
    return false;
  }
  if (rhs_unary != NULL)
  {
    printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", rhs_unary, line);
    return false;
  }

  if (expr->operator == OPERATOR_PLUS)
    *result = val_lhs + val_rhs;
  else if (expr->operator == OPERATOR_MINUS)
    *result = val_lhs - val_rhs;
  else if (expr->operator == OPERATOR_ASTERISK)
    *result = val_lhs * val_rhs;
  else if (expr->operator == OPERATOR_DIV)
  {
    if (val_rhs == 0)
    {
      printf("**ERROR: Divided by 0 happened.\n");
      return false;
    }

    *result = val_lhs / val_rhs;
  }
  else if (expr->operator == OPERATOR_MOD)
    *result = val_lhs % val_rhs;
  else if (expr->operator == OPERATOR_POWER)
  {
    int cur = 1;

    if (val_rhs == 0)
      *result = cur;
    else if (val_rhs > 0)
    {
      cur = val_lhs;

      // multiply val_lhs by itself val_rhs times
      for (int i = 1; i < val_rhs; i++)
      {
        cur *= val_lhs;
      }

      *result = cur;
    }
    else // cannot handle negative exponent since it would produce a float, not an integer
    {
      return false;
    }
  }
  else // a type of operator that isn't handled
    return false;

  return true;
}

static char* retrieve_value(struct UNARY_EXPR* term, int* value, struct RAM* memory)
{
  struct ELEMENT* element = term->element;
  char* element_value = element->element_value;

  if (element->element_type == ELEMENT_INT_LITERAL)
    *value = atoi(element_value);
  else if (element->element_type == ELEMENT_IDENTIFIER)
  {
    struct RAM_VALUE* var_ram_value = ram_read_cell_by_name(memory, element_value);

    if (var_ram_value == NULL)
      return element_value;

    *value = var_ram_value->types.i;
  }

  return NULL;
}

static bool execute_function_call(struct STMT* stmt, struct RAM* memory)
{
  if (stmt->stmt_type != STMT_FUNCTION_CALL) 
    return false;

  struct ELEMENT* parameter = stmt->types.function_call->parameter;

  if (parameter != NULL) // prints parameter of print() function
  {
    char* element_value = parameter->element_value;

    if (parameter->element_type == ELEMENT_STR_LITERAL) // print string (like "print('Hello')")
      printf("%s", element_value);
    else if (parameter->element_type == ELEMENT_INT_LITERAL) // print integer (like "print(1)")
      printf("%d", atoi(element_value));
    else if (parameter->element_type == ELEMENT_IDENTIFIER) // print variable (like "print(x)")
    {
      struct RAM_VALUE* var_ram_value = ram_read_cell_by_name(memory, element_value);

      if (var_ram_value == NULL) // variable does not exist in memory
      {
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", element_value, stmt->line);
        return false;
      }

      int var_value = var_ram_value->types.i; // assuming that all values returned from memory are integers

      printf("%d", var_value);
    }
    else // a type of parameter that isn't handled
      return false;
  }
    
  printf("\n"); // only prints newline character when parameter is NULL

  return true;
}