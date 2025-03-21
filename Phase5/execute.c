/*execute.c*/

//
// Executes nuPython program, given as a Program Graph.
// 
// Solution by Prof. Joe Hummel
// Edited by Prof. Yiji Zhang
//
// Modified by Jad Dibs
//

// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>
#include <math.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"

enum ASGNMT_TYPES
{
  ASGNMT_INT = 0,
  ASGNMT_REAL,
  ASGNMT_STRING,
  ASGNMT_BOOL
};

// Return value of numerous helper functions, used to manage assignments and variables
struct ASGNMT_VALUE
{
  int asgnmt_type; // enum ASGNMT_TYPES

  bool success;

  union
  {
    int i; // for ints and bools
    double d;
    char* s;
  } types;
};

//
// Private functions:
//
static bool execute_function_call(struct STMT* stmt, struct RAM* memory);
static struct ASGNMT_VALUE execute_get_var_value(struct RAM_VALUE* ram_value);
static struct ASGNMT_VALUE execute_get_value(struct UNARY_EXPR* unary, struct STMT* stmt, struct RAM* memory);
static struct ASGNMT_VALUE execute_binary_expression(struct ASGNMT_VALUE lhs, int operator, struct ASGNMT_VALUE rhs, int line);
static struct ASGNMT_VALUE execute_binary_expression_ints(int lhs, int operator, int rhs, int line);
static struct ASGNMT_VALUE execute_binary_expression_reals(double lhs, int operator, double rhs, int line);
static struct ASGNMT_VALUE execute_binary_expression_int_real(int lhs, int operator, double rhs, int line);
static struct ASGNMT_VALUE execute_binary_expression_real_int(double lhs, int operator, int rhs, int line);
static struct ASGNMT_VALUE execute_binary_expression_strings(char* lhs, int operator, char* rhs);
static bool execute_assignment(struct STMT* stmt, struct RAM* memory);
static bool execute_assignment_func(struct STMT* stmt, struct RAM* memory, struct STMT_ASSIGNMENT* assign, struct RAM_VALUE* ram_value);
static bool execute_assignment_expr(struct STMT* stmt, struct RAM* memory, struct STMT_ASSIGNMENT* assign, struct RAM_VALUE* ram_value);

//
// execute_function_call
//
// Executes a function call statement, returning true if 
// successful and false if not (an error message will be
// output before false is returned, so the caller doesn't
// need to output anything).
// 
// Examples: print()
//           print(x)
//           print(123)
//
static bool execute_function_call(struct STMT* stmt, struct RAM* memory)
{
  struct STMT_FUNCTION_CALL* call = stmt->types.function_call;

  //
  // for now we are assuming it's a call to print:
  //
  char* function_name = call->function_name;

  assert(strcmp(function_name, "print") == 0);

  if (call->parameter == NULL)
    printf("\n");
  else 
  {
    //
    // we have a parameter, which type of parameter?
    // Note that a parameter is a simple element, i.e.
    // identifier or literal (or True, False, None):
    //
    char* element_value = call->parameter->element_value;

    if (call->parameter->element_type == ELEMENT_STR_LITERAL) 
    {
      printf("%s\n", element_value);
    }
    else if (call->parameter->element_type == ELEMENT_INT_LITERAL) 
    {
      char* literal = element_value;
      int i = atoi(literal);
      printf("%d\n", i);
    }
    else if (call->parameter->element_type == ELEMENT_REAL_LITERAL)
    {
      char* literal = element_value;
      double i = atof(literal);
      printf("%f\n", i);
    }
    else if (call->parameter->element_type == ELEMENT_TRUE)
    {
      printf("True\n");
    }
    else if (call->parameter->element_type == ELEMENT_FALSE)
    {
      printf("False\n");
    }
    else 
    {
      //
      // we have an identifer => variable
      //
      assert(call->parameter->element_type == ELEMENT_IDENTIFIER);

      char* var_name = element_value;
      struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name);

      if (value == NULL) 
      {
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
        return false;
      }

      if (value->value_type == RAM_TYPE_INT)
        printf("%d\n", value->types.i);
      else if (value->value_type == RAM_TYPE_REAL)
        printf("%f\n", value->types.d);
      else if (value->value_type == RAM_TYPE_STR)
        printf("%s\n", value->types.s);
      else if (value->value_type == RAM_TYPE_BOOLEAN)
      {
        if (value->types.i == 1)
          printf("True\n");
        else
          printf("False\n");
      }
    }
  }

  return true;
}


//
// execute_get_var_value
//
// Given a defined variable in the form of struct RAM_VALUE*, 
// returns the value that it represents in a struct ASGNMT_VALUE.
// The value of the variable is the active member of the types union in struct ASGNMT_VALUE.
//
static struct ASGNMT_VALUE execute_get_var_value(struct RAM_VALUE* ram_value)
{
  struct ASGNMT_VALUE res;

  if (ram_value->value_type == RAM_TYPE_INT) 
  {
    res.asgnmt_type = ASGNMT_INT;
    res.success = true;
    res.types.i = ram_value->types.i;
  }
  else if (ram_value->value_type == RAM_TYPE_REAL)
  {
    res.asgnmt_type = ASGNMT_REAL;
    res.success = true;
    res.types.d = ram_value->types.d;
  }
  else if (ram_value->value_type == RAM_TYPE_STR)
  {
    res.asgnmt_type = ASGNMT_STRING;
    res.success = true;
    res.types.s = ram_value->types.s;
  }
  else if (ram_value->value_type == RAM_TYPE_BOOLEAN)
  {
    res.asgnmt_type = ASGNMT_BOOL;
    res.success = true;

    if (ram_value->types.i == 1)
      res.types.i = 1;
    else
      res.types.i = 0;
  }
  else
  {
    // doesn't handle other RAM value types
    res.success = false;
  }

  return res;
}


//
// execute_get_value
//
// Given a unary expr, returns the value that it represents in a struct ASGNMT_VALUE.
// The value of the unary expr is the active member of the types union in struct ASGNMT_VALUE.
// 
// Note that this function can fail --- success or failure is
// returned as a member of struct ASGNMT_VALUE,
// where True or False is "returned" to denote success/failure.
//
// Why would it fail? If the identifier does not exist in 
// memory. This is a semantic error, and an error message is 
// output before returning.
//
static struct ASGNMT_VALUE execute_get_value(struct UNARY_EXPR* unary, struct STMT* stmt, struct RAM* memory)
{
  //
  // we only have simple elements so far (no unary operators):
  //
  assert(unary->expr_type == UNARY_ELEMENT);

  struct ASGNMT_VALUE res;
  struct ELEMENT* element = unary->element;

  if (element->element_type == ELEMENT_INT_LITERAL) 
  {
    char* literal = element->element_value;

    res.asgnmt_type = ASGNMT_INT;
    res.success = true;
    res.types.i = atoi(literal);
  }
  else if (element->element_type == ELEMENT_REAL_LITERAL)
  {
    char* literal = element->element_value;

    res.asgnmt_type = ASGNMT_REAL;
    res.success = true;
    res.types.d = atof(literal);
  }
  else if (element->element_type == ELEMENT_STR_LITERAL)
  {
    char* literal = element->element_value;

    res.asgnmt_type = ASGNMT_STRING;
    res.success = true;
    res.types.s = literal;
  }
  else if (element->element_type == ELEMENT_TRUE)
  {
    res.asgnmt_type = ASGNMT_BOOL;
    res.success = true;
    res.types.i = 1;
  }
  else if (element->element_type == ELEMENT_FALSE)
  {
    res.asgnmt_type = ASGNMT_BOOL;
    res.success = true;
    res.types.i = 0;
  }
  else 
  {
    //
    // identifier => variable
    //
    assert(element->element_type == ELEMENT_IDENTIFIER);

    char* var_name = element->element_value;

    struct RAM_VALUE* ram_value = ram_read_cell_by_name(memory, var_name);

    if (ram_value == NULL) 
    {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
      res.success = false;
    }
    else 
    {
      res = execute_get_var_value(ram_value);
    }
  }

  //
  // success/failure has already been "returned", so
  // return value (or -1 will be returned if failed):
  //
  return res;
}


//
// execute_binary_expression
//
// Given two values and an operator, performs the operation
// and returns the result.
//
static struct ASGNMT_VALUE execute_binary_expression(struct ASGNMT_VALUE lhs, int operator, struct ASGNMT_VALUE rhs, int line)
{
  assert(operator != OPERATOR_NO_OP);

  struct ASGNMT_VALUE result;
  
  if (lhs.asgnmt_type == ASGNMT_INT && rhs.asgnmt_type == ASGNMT_INT)
    result = execute_binary_expression_ints(lhs.types.i, operator, rhs.types.i, line);
  else if (lhs.asgnmt_type == ASGNMT_REAL && rhs.asgnmt_type == ASGNMT_REAL)
    result = execute_binary_expression_reals(lhs.types.d, operator, rhs.types.d, line);
  else if (lhs.asgnmt_type == ASGNMT_INT && rhs.asgnmt_type == ASGNMT_REAL)
    result = execute_binary_expression_int_real(lhs.types.i, operator, rhs.types.d, line);
  else if (lhs.asgnmt_type == ASGNMT_REAL && rhs.asgnmt_type == ASGNMT_INT)
    result = execute_binary_expression_real_int(lhs.types.d, operator, rhs.types.i, line);
  else if (lhs.asgnmt_type == ASGNMT_STRING && rhs.asgnmt_type == ASGNMT_STRING)
    result = execute_binary_expression_strings(lhs.types.s, operator, rhs.types.s);
  else
  {
    printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
    result.success = 0;
  }
  
  return result;
}


//
// execute_binary_expression_ints
//
// Given two ints and an operator, performs the operation
// and returns the result.
//
static struct ASGNMT_VALUE execute_binary_expression_ints(int lhs, int operator, int rhs, int line)
{
  assert(operator != OPERATOR_NO_OP);
  
  struct ASGNMT_VALUE result;
  result.success = 0;
  //
  // perform the operation:
  //
  switch (operator)
  {
  case OPERATOR_PLUS:
    result.asgnmt_type = ASGNMT_INT;
    result.success = 1;
    result.types.i = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    result.asgnmt_type = ASGNMT_INT;
    result.success = 1;
    result.types.i = lhs - rhs;
    break;

  case OPERATOR_ASTERISK:
    result.asgnmt_type = ASGNMT_INT;
    result.success = 1;
    result.types.i = lhs * rhs;
    break;

  case OPERATOR_POWER:
    result.asgnmt_type = ASGNMT_INT;
    result.success = 1;
    result.types.i = (int)pow(lhs, rhs);
    break;

  case OPERATOR_MOD:
    result.asgnmt_type = ASGNMT_INT;
    result.success = 1;
    result.types.i = lhs % rhs;
    break;

  case OPERATOR_DIV:
    result.asgnmt_type = ASGNMT_INT;
    if(rhs != 0)
    {
      result.success = 1;
      result.types.i = lhs / rhs;
    }

      
    else
    {
      printf("**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = 0;
    }
    break;

  case OPERATOR_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs == rhs;
    break;

  case OPERATOR_NOT_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs != rhs;
    break;

  case OPERATOR_LT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs < rhs;
    break;

  case OPERATOR_LTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs <= rhs;
    break;

  case OPERATOR_GT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs > rhs;
    break;

  case OPERATOR_GTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs >= rhs;
    break;

  default:
    //
    // did we miss something?
    //
    printf("**INTERNAL ERROR: unexpected operator (%d) in execute_binary_expr\n", operator);
    assert(false);
  }

  return result;
}


//
// execute_binary_expression_reals
//
// Given two reals and an operator, performs the operation
// and returns the result.
//
static struct ASGNMT_VALUE execute_binary_expression_reals(double lhs, int operator, double rhs, int line)
{
  assert(operator != OPERATOR_NO_OP);
  
  struct ASGNMT_VALUE result;
  result.success = 0;
  //
  // perform the operation:
  //
  switch (operator)
  {
  case OPERATOR_PLUS:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs - rhs;
    break;

  case OPERATOR_ASTERISK:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs * rhs;
    break;

  case OPERATOR_POWER:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = pow(lhs, rhs);
    break;

  case OPERATOR_MOD:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = fmod(lhs, rhs);
    break;

  case OPERATOR_DIV:
    result.asgnmt_type = ASGNMT_REAL;
    if(rhs != 0.0)
    {
      result.success = 1;
      result.types.d = lhs / rhs;
    }

      
    else
    {
      printf("**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = 0;
    }
    break;

  case OPERATOR_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs == rhs;
    break;

  case OPERATOR_NOT_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs != rhs;
    break;

  case OPERATOR_LT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs < rhs;
    break;

  case OPERATOR_LTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs <= rhs;
    break;

  case OPERATOR_GT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs > rhs;
    break;

  case OPERATOR_GTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs >= rhs;
    break;

  default:
    //
    // did we miss something?
    //
    printf("**INTERNAL ERROR: unexpected operator (%d) in execute_binary_expr\n", operator);
    assert(false);
  }

  return result;
}


//
// execute_binary_expression_int_real
//
// Given lhs as int, rhs as real, and an operator, performs the operation
// and returns the result.
//
static struct ASGNMT_VALUE execute_binary_expression_int_real(int lhs, int operator, double rhs, int line)
{
  assert(operator != OPERATOR_NO_OP);
  
  struct ASGNMT_VALUE result;
  result.success = 0;
  //
  // perform the operation:
  //
  switch (operator)
  {
  case OPERATOR_PLUS:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs - rhs;
    break;

  case OPERATOR_ASTERISK:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs * rhs;
    break;

  case OPERATOR_POWER:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = pow(lhs, rhs);
    break;

  case OPERATOR_MOD:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = fmod(lhs, rhs);
    break;

  case OPERATOR_DIV:
    result.asgnmt_type = ASGNMT_REAL;
    if(rhs != 0.0 && rhs != 0)
    {
      result.success = 1;
      result.types.d = lhs / rhs;
    }

      
    else
    {
      printf("**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = 0;
    }
    break;

  case OPERATOR_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs == rhs;
    break;

  case OPERATOR_NOT_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs != rhs;
    break;

  case OPERATOR_LT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs < rhs;
    break;

  case OPERATOR_LTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs <= rhs;
    break;

  case OPERATOR_GT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs > rhs;
    break;

  case OPERATOR_GTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs >= rhs;
    break;

  default:
    //
    // did we miss something?
    //
    printf("**INTERNAL ERROR: unexpected operator (%d) in execute_binary_expr\n", operator);
    assert(false);
  }

  return result;
}


//
// execute_binary_expression_real_int
//
// Given lhs as real, rhs as int, and an operator, performs the operation
// and returns the result.
//
static struct ASGNMT_VALUE execute_binary_expression_real_int(double lhs, int operator, int rhs, int line)
{
  assert(operator != OPERATOR_NO_OP);
  
  struct ASGNMT_VALUE result;
  result.success = 0;
  //
  // perform the operation:
  //
  switch (operator)
  {
  case OPERATOR_PLUS:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs - rhs;
    break;

  case OPERATOR_ASTERISK:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = lhs * rhs;
    break;

  case OPERATOR_POWER:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = pow(lhs, rhs);
    break;

  case OPERATOR_MOD:
    result.asgnmt_type = ASGNMT_REAL;
    result.success = 1;
    result.types.d = fmod(lhs, rhs);
    break;

  case OPERATOR_DIV:
    result.asgnmt_type = ASGNMT_REAL;
    if(rhs != 0.0 && rhs != 0)
    {
      result.success = 1;
      result.types.d = lhs / rhs;
    }

      
    else
    {
      printf("**ZeroDivisionError: division by zero (line %d)\n", line);
      result.success = 0;
    }
    break;

  case OPERATOR_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs == rhs;
    break;

  case OPERATOR_NOT_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs != rhs;
    break;

  case OPERATOR_LT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs < rhs;
    break;

  case OPERATOR_LTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs <= rhs;
    break;

  case OPERATOR_GT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs > rhs;
    break;

  case OPERATOR_GTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    result.types.i = lhs >= rhs;
    break;

  default:
    //
    // did we miss something?
    //
    printf("**INTERNAL ERROR: unexpected operator (%d) in execute_binary_expr\n", operator);
    assert(false);
  }

  return result;
}


//
// execute_binary_expression_strings
//
// Given two strings and an operator, performs the operation
// and returns the result.
//
static struct ASGNMT_VALUE execute_binary_expression_strings(char* lhs, int operator, char* rhs)
{
  assert(operator != OPERATOR_NO_OP);
  
  struct ASGNMT_VALUE result;
  result.success = 0;

  int compare_val = strcmp(lhs, rhs);

  //
  // perform the operation:
  //
  switch (operator)
  {
  case OPERATOR_PLUS:
    result.asgnmt_type = ASGNMT_STRING;
    result.success = 1;

    char* concatenated = malloc((strlen(lhs) + strlen(rhs) + 1) * sizeof(char));

    strcpy(concatenated, lhs);
    strcat(concatenated, rhs);

    result.types.s = concatenated;
    break;

  case OPERATOR_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;

    if (compare_val == 0)
      result.types.i = 1;
    else
      result.types.i = 0;

    break;

  case OPERATOR_NOT_EQUAL:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    
    if (compare_val != 0)
      result.types.i = 1;
    else
      result.types.i = 0;

    break;

  case OPERATOR_LT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;

    if (compare_val < 0)
      result.types.i = 1;
    else
      result.types.i = 0;

    break;

  case OPERATOR_LTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;
    
    if (compare_val <= 0)
      result.types.i = 1;
    else
      result.types.i = 0;

    break;

  case OPERATOR_GT:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;

    if (compare_val > 0)
      result.types.i = 1;
    else
      result.types.i = 0;

    break;

  case OPERATOR_GTE:
    result.asgnmt_type = ASGNMT_BOOL;
    result.success = 1;

    if (compare_val >= 0)
      result.types.i = 1;
    else
      result.types.i = 0;

    break;

  default:
    //
    // did we miss something?
    //
    printf("**INTERNAL ERROR: unexpected operator (%d) in execute_binary_expr\n", operator);
    assert(false);
  }

  return result;
}


//
// execute_assignment
//
// Executes an assignment statement, returning true if 
// successful and false if not (an error message will be
// output before false is returned, so the caller doesn't
// need to output anything).
// 
// Examples: x = 123
//           y = x ** 2
//
static bool execute_assignment(struct STMT* stmt, struct RAM* memory)
{
  struct STMT_ASSIGNMENT* assign = stmt->types.assignment;

  char* var_name = assign->var_name;

  //
  // no pointers yet:
  //
  assert(assign->isPtrDeref == false);

  bool success;
  struct RAM_VALUE ram_value;

  if (assign->rhs->value_type == VALUE_FUNCTION_CALL)
  {
    bool func_success = execute_assignment_func(stmt, memory, assign, &ram_value);

    if (!func_success)
      return false;
  }
  else // assign->rhs->value_type == VALUE_EXPR
  {
    assert(assign->rhs->value_type == VALUE_EXPR);

    bool expr_success = execute_assignment_expr(stmt, memory, assign, &ram_value);

    if (!expr_success)
      return false;
  }

  //
  // write the value to memory:
  //

  success = ram_write_cell_by_name(memory, ram_value, var_name);

  return success;
}


//
// execute_assignment_func
//
// Executes an assignment statement whose right hand side is a function, 
// returning true if successful and false if not.
//
static bool execute_assignment_func(struct STMT* stmt, struct RAM* memory, struct STMT_ASSIGNMENT* assign, struct RAM_VALUE* ram_value)
{
  struct FUNCTION_CALL* func = assign->rhs->types.function_call;

  char* func_name = func->function_name;
  struct ELEMENT* param = func->parameter;

  if (strcmp(func_name, "input") == 0)
  {
    assert(param->element_type == ELEMENT_STR_LITERAL);

    printf("%s", param->element_value);

    char line[256];

    fgets(line, sizeof(line), stdin);

    // delete EOL chars from input: 
    line[strcspn(line, "\r\n")] = '\0';

    char* user_input = malloc(sizeof(line));
    strcpy(user_input, line);

    ram_value->value_type = RAM_TYPE_STR;
    ram_value->types.s = user_input;
  }
  else if (strcmp(func_name, "int") == 0)
  {
    struct RAM_VALUE* var_str = ram_read_cell_by_name(memory, param->element_value);
    assert(var_str->value_type == RAM_TYPE_STR);
    char* var_str_val = var_str->types.s;

    int var_int = atoi(var_str_val);

    if (var_int == 0 && var_str_val[0] != '0')
    {
      printf("**SEMANTIC ERROR: invalid string for int() (line %d)\n", stmt->line);
      return false;
    }
    else
    {
      ram_value->value_type = RAM_TYPE_INT;
      ram_value->types.i = var_int;
    }
  }
  else if (strcmp(func_name, "float") == 0)
  {
    struct RAM_VALUE* var_str = ram_read_cell_by_name(memory, param->element_value);
    assert(var_str->value_type == RAM_TYPE_STR);
    char* var_str_val = var_str->types.s;

    double var_float = atof(var_str_val);

    if (var_float == 0 && var_str_val[0] != '0')
    {
      printf("**SEMANTIC ERROR: invalid string for float() (line %d)\n", stmt->line);
      return false;
    }
    else
    {
      ram_value->value_type = RAM_TYPE_REAL;
      ram_value->types.d = var_float;
    }
  }
  else
    return false;

  return true;
}


//
// execute_assignment_expr
//
// Executes an assignment statement whose right hand side is an expression, 
// returning true if successful and false if not.
//
static bool execute_assignment_expr(struct STMT* stmt, struct RAM* memory, struct STMT_ASSIGNMENT* assign, struct RAM_VALUE* ram_value)
{
  struct EXPR* expr = assign->rhs->types.expr;

  //
  // we always have a LHS:
  //
  assert(expr->lhs != NULL);

  struct ASGNMT_VALUE lhs_value = execute_get_value(expr->lhs, stmt, memory);

  if (!lhs_value.success)  // semantic error? If so, return now:
    return false;

  //
  // do we have a binary expression?
  //
  if (!expr->isBinaryExpr) 
  {
    if (lhs_value.asgnmt_type == ASGNMT_INT)
    {
      ram_value->value_type = RAM_TYPE_INT;
      ram_value->types.i = lhs_value.types.i;
    }
    else if (lhs_value.asgnmt_type == ASGNMT_REAL)
    {
      ram_value->value_type = RAM_TYPE_REAL;
      ram_value->types.d = lhs_value.types.d;
    }
    else if (lhs_value.asgnmt_type == ASGNMT_STRING)
    {
      ram_value->value_type = RAM_TYPE_STR;
      ram_value->types.s = lhs_value.types.s;
    }
    else
    {
      assert(lhs_value.asgnmt_type == ASGNMT_BOOL);

      ram_value->value_type = RAM_TYPE_BOOLEAN;
      ram_value->types.i = lhs_value.types.i;
    }
  }
  else 
  {
    //
    // binary expression such as x + y
    //
    assert(expr->operator != OPERATOR_NO_OP);  // we must have an operator

    struct ASGNMT_VALUE rhs_value = execute_get_value(expr->rhs, stmt, memory);

    if (!rhs_value.success)  // semantic error? If so, return now:
      return false;

    //
    // perform the operation:
    //
    struct ASGNMT_VALUE expr_result = execute_binary_expression(lhs_value, expr->operator, rhs_value, stmt->line);

    if(!expr_result.success)
      return false;
    
    if (expr_result.asgnmt_type == ASGNMT_INT)
    {
      ram_value->value_type = RAM_TYPE_INT;
      ram_value->types.i = expr_result.types.i;
    }
    else if (expr_result.asgnmt_type == ASGNMT_REAL)
    {
      ram_value->value_type = RAM_TYPE_REAL;
      ram_value->types.d = expr_result.types.d;
    }
    else if (expr_result.asgnmt_type == ASGNMT_STRING)
    {
      ram_value->value_type = RAM_TYPE_STR;
      ram_value->types.s = expr_result.types.s;
    }
    else if (expr_result.asgnmt_type == ASGNMT_BOOL)
    {
      ram_value->value_type = RAM_TYPE_BOOLEAN;
      ram_value->types.i = expr_result.types.i;
    }
  }

  return true;
}


//
// Public functions:
//

//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// an error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory)
{
  struct STMT* stmt = program;

  //
  // traverse through the program statements:
  //
  while (stmt != NULL) 
  {

    if (stmt->stmt_type == STMT_ASSIGNMENT) 
    {

      bool success = execute_assignment(stmt, memory);

      if (!success)
        return;

      stmt = stmt->types.assignment->next_stmt;  // advance
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

      //
      // nothing to do!
      //

      stmt = stmt->types.pass->next_stmt;
    }
  }//while

  //
  // done:
  //
  return;
}
