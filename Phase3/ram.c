/*ram.c*/

//
// Random access memory (RAM) for nuPython debugger to simulate the reading and writing of variables
//
// Jad Dibs
//
// Template: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // true, false
#include <string.h>
#include <assert.h>

#include "ram.h"

//
// Helper functions
//

//
// dup_string
//
// Returns duplicate of input string s
//
static char* dup_string(char* s);

//
// reallocate_memory
//
// Reallocates memory when memory is full (num_values equals capacity) by copying input memory and doubling capacity
//
static void reallocate_memory(struct RAM* memory);


//
// Public functions:
//

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None.
//
struct RAM* ram_init(void)
{
  // initializes dynamically-allocated memory
  struct RAM* memory = (struct RAM*) malloc(sizeof(struct RAM));

  // checks if there is enough space in memory for memory
  if (memory == NULL)
  {
    exit(0);
  }

  // initializes struct RAM fields of memory
  memory->num_values = 0;
  memory->capacity = 4;
  memory->cells = (struct RAM_CELL*) malloc(memory->capacity * sizeof(struct RAM_CELL));

  if (memory->cells == NULL)
  {
    ram_destroy(memory);
    exit(0);
  }

  // initializes memory cells to None by making all fields of RAM_CELL null, none, or 0
  for (int i = 0; i < memory->capacity; i++)
  {
    memory->cells[i].identifier = NULL;
    memory->cells[i].value.value_type = RAM_TYPE_NONE;
  }

  return memory;
}


//
// ram_destroy
//
// Frees the dynamically-allocated memory associated with
// the given memory. After the call returns, you cannot
// use the memory.
//
void ram_destroy(struct RAM* memory)
{
  for (int i = 0; i < memory->capacity; i++)
  {
    free(memory->cells[i].identifier);

    if (memory->cells[i].value.value_type == RAM_TYPE_STR)
      free(memory->cells[i].value.types.s);
  }
  
  free(memory->cells);
  free(memory);
}


//
// ram_get_addr
// 
// If the given identifier (e.g. "x") has been written to 
// memory by name, returns the address of this value --- an integer
// in the range 0..N-1 where N is the number of values currently 
// stored in memory. Returns -1 if no such identifier exists 
// in memory. 
// 
// NOTE: a variable has to be written to memory by name before you can
// get its address. Once a variable is written to memory, its
// address never changes.
//
int ram_get_addr(struct RAM* memory, char* identifier)
{
  for (int i = 0; i < memory->num_values; i++)
  {
    char* ident_in_memory = memory->cells[i].identifier;

    if (ident_in_memory != NULL && strcmp(ident_in_memory, identifier) == 0)
    {
      return i;
    }
  }

  return -1;
}


//
// ram_read_cell_by_addr
//
// Given a memory address (an integer in the range 0..N-1), 
// returns a COPY of the value contained in that memory cell.
// Returns NULL if the address is not valid.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
// NOTE: a variable has to be written to memory by name before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address)
{
  for (int i = 0; i < memory->num_values; i++)
  {
    if (i == address)
    {
      struct RAM_VALUE* value = (struct RAM_VALUE*) malloc(sizeof(struct RAM_VALUE));

      if (value == NULL)
      {
        exit(0);
      }

      //
      // need to duplicate char* if value type is string to prevent copying pointer
      //
      if (memory->cells[i].value.value_type == RAM_TYPE_STR)
      {
        value->value_type = RAM_TYPE_STR;
        value->types.s = dup_string(memory->cells[i].value.types.s);
      }
      //
      // no need to duplicate other types since not pointers
      //
      else
      {
        *value = memory->cells[i].value;
      }

      return value;
    }
  }

  return NULL;
}


// 
// ram_read_cell_by_name
//
// If the given name (e.g. "x") has been written to 
// memory, returns a COPY of the value contained in memory.
// Returns NULL if no such name exists in memory.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* name)
{
  for (int i = 0; i < memory->capacity; i++)
  {
    //
    // creates copy if value contained in memory
    //
    if (memory->cells[i].identifier != NULL && strcmp(name, memory->cells[i].identifier) == 0)
    {
      struct RAM_VALUE* value = (struct RAM_VALUE*) malloc(sizeof(struct RAM_VALUE));

      if (value == NULL)
      {
        exit(0);
      }

      //
      // need to duplicate char* if value type is string to prevent copying pointer
      //
      if (memory->cells[i].value.value_type == RAM_TYPE_STR)
      {
        value->value_type = RAM_TYPE_STR;
        value->types.s = dup_string(memory->cells[i].value.types.s);
      }
      //
      // no need to duplicate other types since not pointers
      //
      else
      {
        *value = memory->cells[i].value;
      }

      return value;
    }
  }
  
  //
  // value not contained in memory
  //
  return NULL;
}


//
// ram_free_value
//
// Frees the memory value returned by ram_read_cell_by_name and
// ram_read_cell_by_addr.
//
void ram_free_value(struct RAM_VALUE* value)
{
  if (value->value_type == RAM_TYPE_STR)
  {
    free(value->types.s);
  }

  free(value);
}


//
// ram_write_cell_by_addr
//
// Writes the given value to the memory cell at the given 
// address. If a value already exists at this address, that
// value is overwritten by this new value. Returns true if 
// the value was successfully written, false if not (which 
// implies the memory address is invalid).
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory by name before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address)
{
  if (address >= memory->num_values || address < 0)
    return false;

  // ensure not to leave old string dangling
  if (memory->cells[address].value.value_type == RAM_TYPE_STR)
  {
    free(memory->cells[address].value.types.s);
  }

  // need to duplicate char* if value type is string to prevent copying pointer
  if (value.value_type == RAM_TYPE_STR)
  {
    memory->cells[address].value.value_type = RAM_TYPE_STR;
    memory->cells[address].value.types.s = dup_string(value.types.s);
  }
  // no need to duplicate other types since not pointers
  else
  {
    memory->cells[address].value = value;
  }

  return true;
}


//
// ram_write_cell_by_name
//
// Writes the given value to a memory cell named by the given
// name. If a memory cell already exists with this name, the
// existing value is overwritten by this new value. Returns
// true since this operation always succeeds.
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory by name before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* name)
{
  int address = ram_get_addr(memory, name);
  
  if (address != -1) // overwrite cell
  {
    // ensure not to leave old string dangling
    if (memory->cells[address].value.value_type == RAM_TYPE_STR)
    {
      free(memory->cells[address].value.types.s);
    }

    // need to duplicate char* if value type is string to prevent copying pointer
    if (value.value_type == RAM_TYPE_STR)
    {
      memory->cells[address].value.value_type = RAM_TYPE_STR;
      memory->cells[address].value.types.s = dup_string(value.types.s);
    }
    // no need to duplicate other types since not pointers
    else
    {
      memory->cells[address].value = value;
    }    
  }
  else // new cell
  {
    // find open position in memory cells
    for (int i = 0; i < memory->capacity; i++)
    {
      if (memory->cells[i].identifier == NULL) // open cell
      {
        memory->cells[i].identifier = dup_string(name);

        // need to duplicate char* if value type is string to prevent copying pointer
        if (value.value_type == RAM_TYPE_STR)
        {
          memory->cells[i].value.value_type = RAM_TYPE_STR;
          memory->cells[i].value.types.s = dup_string(value.types.s);
        }
        // no need to duplicate other types since not pointers
        else
        {
          memory->cells[i].value = value;
        }

        memory->num_values += 1;

        return true;
      }
    }

    //
    // num values have reached capacity
    //
    reallocate_memory(memory);
    int idx = memory->num_values;

    memory->num_values += 1;
    memory->cells[idx].identifier = dup_string(name);

    // need to duplicate char* if value type is string to prevent copying pointer
    if (value.value_type == RAM_TYPE_STR)
    {
      memory->cells[idx].value.value_type = RAM_TYPE_STR;
      memory->cells[idx].value.types.s = dup_string(value.types.s);
    }
    // no need to duplicate other types since not pointers
    else
    {
      memory->cells[idx].value = value;
    }
  }
  
  return true;
}

//
// ram_print
//
// Prints the contents of memory to the console.
//
void ram_print(struct RAM* memory)
{
  printf("**MEMORY PRINT**\n");

  printf("Capacity: %d\n", memory->capacity);
  printf("Num values: %d\n", memory->num_values);
  printf("Contents:\n");

  for (int i = 0; i < memory->num_values; i++)
  {
    char* var_name = memory->cells[i].identifier;
    int val_type = memory->cells[i].value.value_type;

    if (var_name != NULL) printf(" %d: %s, ", i, var_name);

    if (val_type == RAM_TYPE_INT)
    {
      int val = memory->cells[i].value.types.i;
      printf("int, %d", val);
    }
    else if (val_type == RAM_TYPE_REAL)
    {
      double val = memory->cells[i].value.types.d;
      printf("real, %lf", val);
    }
    else if (val_type == RAM_TYPE_STR)
    {
      char* val = memory->cells[i].value.types.s;
      printf("str, '%s'", val);
    }
    else if (val_type == RAM_TYPE_PTR)
    {
      int val = memory->cells[i].value.types.i;
      printf("ptr, %d", val);
    }
    else if (val_type == RAM_TYPE_BOOLEAN)
    {
      int val = memory->cells[i].value.types.i;

      if (val == 0)
      {
        printf("boolean, False");
      }
      else
      {
        printf("boolean, True");
      }
    }
    else
    {
      printf("none, None");
    }

    printf("\n");
  }

  printf("**END PRINT**\n");
}


//
// Helper functions
//

static char* dup_string(char* s)
{
  if (s == NULL) return NULL;

  size_t len = strlen(s) + 1;

  char* copy = (char*) malloc(len * sizeof(char));

  if (copy == NULL) {
    printf("**ERROR: out of memory\n");
    exit(0);
  }

  strcpy(copy, s);  // copy the chars in s:
  return copy;
}


static void reallocate_memory(struct RAM* memory)
{
  int prev_cap = memory->capacity;
  int new_cap = memory->capacity * 2;

  struct RAM_CELL* new_cells = (struct RAM_CELL*) realloc(memory->cells, new_cap * sizeof(struct RAM_CELL));

  // checks if there is enough space in memory for new memory->cells
  if (new_cells == NULL)
  {
    exit(0);
  }

  // reallocate memory in memory
  memory->capacity = new_cap;
  memory->cells = new_cells;

  // initialize new cells
  for (int i = prev_cap; i < new_cap; i++)
  {
    memory->cells[i].identifier = NULL;
    memory->cells[i].value.value_type = RAM_TYPE_NONE;
  }
}
