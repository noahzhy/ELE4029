/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* SIZE is the size of the hash table */
#define SIZE 211

#include "globals.h"

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
{
  int lineno;
  struct LineListRec *next;
} * LineList;

typedef struct BucketListRec
{
  char *name;
  TreeNode *treeNode;
  LineList lines;
  int memloc;
  struct BucketListRec *next;
} * BucketList;

typedef struct ScopeListRec
{
  char *funcName;
  BucketList hashTable[SIZE];
  struct ScopeListRec *parent;
  int nestedLevel;
} * ScopeList;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char *name, int lineno, int loc, TreeNode *treeNode);

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup(char *name);
void st_add_lineno(char *name, int lineno);
int st_lookup_top(char *name);

BucketList get_bucket(char *name);

/* Stack for static scope */
ScopeList sc_create(char *funcName);
ScopeList sc_top(void);
void sc_pop(void);
void sc_push(ScopeList scope);
int addLocation(void);

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE *listing);
void print_SymTab(FILE *listing);
void print_FuncTab(FILE *listing);
void print_Func_globVar(FILE *listing);
void print_FuncP_N_LoclVar(FILE *listing);

#endif
