/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "globals.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4
#define MAX_SCOPES 1000

/* the hash function */
static int hash(char *key)
{
  int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  {
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

ScopeList scopes[MAX_SCOPES], scopeStack[MAX_SCOPES];
int cntScope = 0, cntScopeStack = 0, location[MAX_SCOPES];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char *name, int lineno, int loc, TreeNode *treeNode)
{
  int h = hash(name);
  ScopeList nowScope = sc_top();
  BucketList l = nowScope->hashTable[h];
  while ((l != NULL) && (strcmp(name, l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  {
    l = (BucketList)malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = treeNode;
    l->lines = (LineList)malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = nowScope->hashTable[h];
    nowScope->hashTable[h] = l;
  }
  else /* found in table, so just add line number */
  {
    // LineList t = l->lines;
    // while (t->next != NULL) t = t->next;
    // t->next = (LineList) malloc(sizeof(struct LineListRec));
    // t->next->lineno = lineno;
    // t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup(char *name)
{
  BucketList l = get_bucket(name);
  if (l != NULL)
    return l->memloc;
  return -1;
}

void st_add_lineno(char *name, int lineno)
{
  BucketList bl = get_bucket(name);
  LineList ll = bl->lines;
  while (ll->next != NULL)
    ll = ll->next;
  ll->next = (LineList)malloc(sizeof(struct LineListRec));
  ll->next->lineno = lineno;
  ll->next->next = NULL;
}

int st_lookup_top(char *name)
{
  int h = hash(name);
  ScopeList nowScope = sc_top();
  //while(nowScope != NULL)
  BucketList l = nowScope->hashTable[h];
  while ((l != NULL) && (strcmp(name, l->name) != 0))
    l = l->next;
  if (l != NULL)
    return l->memloc;
  //  nowScope = nowScope->parent;

  return -1;
}

BucketList get_bucket(char *name)
{
  int h = hash(name);
  ScopeList nowScope = sc_top();
  while (nowScope != NULL)
  {
    BucketList l = nowScope->hashTable[h];
    while ((l != NULL) && (strcmp(name, l->name) != 0))
      l = l->next;
    if (l != NULL)
      return l;
    nowScope = nowScope->parent;
  }
  return NULL;
}

/* Stack for static scope */
ScopeList sc_create(char *funcName)
{
  ScopeList newScope;
  newScope = (ScopeList)malloc(sizeof(struct ScopeListRec));
  newScope->funcName = funcName;
  newScope->nestedLevel = cntScopeStack;
  newScope->parent = sc_top();
  scopes[cntScope++] = newScope;

  return newScope;
}

ScopeList sc_top(void)
{
  if (!cntScopeStack)
    return NULL;
  return scopeStack[cntScopeStack - 1];
}

void sc_pop(void)
{
  if (cntScopeStack)
    cntScopeStack--;
}

void sc_push(ScopeList scope)
{
  scopeStack[cntScopeStack] = scope;
  location[cntScopeStack++] = 0;
}

int addLocation(void)
{
  return location[cntScopeStack - 1]++;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */

void printSymTab(FILE *listing)
{
  print_SymTab(listing);
  fprintf(listing, "\n");
  print_FuncTab(listing);
  fprintf(listing, "\n");
  print_Func_globVar(listing);
  fprintf(listing, "\n");
  print_FuncP_N_LoclVar(listing);
} /* printSymTab */

void print_SymTab(FILE *listing)
{
  int i, j;
  fprintf(listing, "\n< Symbol Table >\n");
  fprintf(listing, "Variable Name  Variable Type  Scope Name  Location  Line Numbers\n");
  fprintf(listing, "-------------  -------------  ----------  --------  ------------\n");

  for (i = 0; i < cntScope; i++)
  {
    ScopeList nowScope = scopes[i];
    BucketList *hashTable = nowScope->hashTable;

    for (j = 0; j < SIZE; j++)
    {
      if (hashTable[j] != NULL)
      {
        BucketList bl = hashTable[j];
        TreeNode *node = bl->treeNode;

        while (bl != NULL)
        {
          LineList ll = bl->lines;
          fprintf(listing, "%-15s", bl->name);

          switch (node->nodekind)
          {
          case DeclK:
            switch (node->kind.decl)
            {
            case FuncK:
              fprintf(listing, "%-15s", "Function");
              break;
            case VarK:
              switch (node->type)
              {
              case Void:
                fprintf(listing, "%-15s", "Void");
                break;
              case Integer:
                fprintf(listing, "%-15s", "Integer");
                break;
              default:
                break;
              }
              break;
            case ArrVarK:
              fprintf(listing, "%-15s", "IntegerArray");
              break;
            default:
              break;
            }
            break;
          case ParamK:
            switch (node->kind.param)
            {
            case ArrParamK:
              fprintf(listing, "%-15s", "IntegerArray");
              break;
            case NonArrParamK:
              fprintf(listing, "%-15s", "Integer");
              break;
            default:
              break;
            }
            break;
          default:
            break;
          }

          fprintf(listing, "%-12s", nowScope->funcName);
          fprintf(listing, "%-10d", bl->memloc);
          while (ll != NULL)
          {
            fprintf(listing, "%4d", ll->lineno);
            ll = ll->next;
          }
          fprintf(listing, "\n");

          bl = bl->next;
        }
      }
    }
  }
}

void print_FuncTab(FILE *listing)
{
  int i, j, k, l;
  fprintf(listing, "\n< Function Table >\n");
  fprintf(listing, "Function Name  Scope Name  Return Type  Parameter Name  Parameter Type\n");
  fprintf(listing, "-------------  ----------  -----------  --------------  --------------\n");

  for (i = 0; i < cntScope; i++)
  {
    ScopeList nowScope = scopes[i];
    BucketList *hashTable = nowScope->hashTable;

    for (j = 0; j < SIZE; j++)
    {
      if (hashTable[j] != NULL)
      {
        BucketList bl = hashTable[j];
        TreeNode *node = bl->treeNode;

        while (bl != NULL)
        {
          switch (node->nodekind)
          {
          case DeclK:
            if (node->kind.decl == FuncK) /* Function print */
            {
              fprintf(listing, "%-15s", bl->name);
              fprintf(listing, "%-12s", nowScope->funcName);
              switch (node->type)
              {
              case Void:
                fprintf(listing, "%-13s", "Void");
                break;
              case Integer:
                fprintf(listing, "%-13s", "Integer");
                break;
              default:
                break;
              }

              int noParam = TRUE;
              for (k = 0; k < cntScope; k++)
              {
                ScopeList paramScope = scopes[k];
                if (strcmp(paramScope->funcName, bl->name) != 0)
                  continue;
                BucketList *paramhashTable = paramScope->hashTable; //printf("c\n");

                for (l = 0; l < SIZE; l++)
                {
                  if (paramhashTable[l] != NULL)
                  {
                    BucketList pbl = paramhashTable[l];
                    TreeNode *pnode = pbl->treeNode;

                    while (pbl != NULL)
                    {
                      switch (pnode->nodekind)
                      {
                      case ParamK:
                        noParam = FALSE;
                        fprintf(listing, "\n");
                        fprintf(listing, "%-40s", "");
                        fprintf(listing, "%-16s", pbl->name);
                        switch (pnode->type)
                        {
                        case Integer:
                          fprintf(listing, "%-14s", "Integer");
                          break;
                        case IntegerArray:
                          fprintf(listing, "%-14s", "IntegerArray");
                          break;
                        default:
                          break;
                        }
                        break;
                      default:
                        break;
                      }
                      pbl = pbl->next;
                    }
                  }
                }
                break;
              }
              if (noParam)
              {
                fprintf(listing, "%-16s", "");
                if (strcmp(bl->name, "output") != 0)
                  fprintf(listing, "%-14s", "Void");
                else
                  fprintf(listing, "\n%-56s%-14s", "", "Integer");
              }

              fprintf(listing, "\n");
            }
            break;
          default:
            break;
          }
          bl = bl->next;
        }
      }
    }
  }
}

void print_Func_globVar(FILE *listing)
{
  int i, j;
  fprintf(listing, "\n< Function and Global Variables >\n");
  fprintf(listing, "   ID Name      ID Type    Data Type\n");
  fprintf(listing, "-------------  ---------  -----------\n");

  for (i = 0; i < cntScope; i++)
  {
    ScopeList nowScope = scopes[i];
    if (strcmp(nowScope->funcName, "global") != 0)
      continue;

    BucketList *hashTable = nowScope->hashTable;

    for (j = 0; j < SIZE; j++)
    {
      if (hashTable[j] != NULL)
      {
        BucketList bl = hashTable[j];
        TreeNode *node = bl->treeNode;

        while (bl != NULL)
        {
          switch (node->nodekind)
          {
          case DeclK:
            fprintf(listing, "%-15s", bl->name);
            switch (node->kind.decl)
            {
            case FuncK:
              fprintf(listing, "%-11s", "Function");
              switch (node->type)
              {
              case Void:
                fprintf(listing, "%-11s", "Void");
                break;
              case Integer:
                fprintf(listing, "%-11s", "Integer");
                break;
              default:
                break;
              }
              break;
            case VarK:
              switch (node->type)
              {
              case Void:
                fprintf(listing, "%-11s", "Variable");
                fprintf(listing, "%-11s", "Void");
                break;
              case Integer:
                fprintf(listing, "%-11s", "Variable");
                fprintf(listing, "%-11s", "Integer");
                break;
              default:
                break;
              }
              break;
            case ArrVarK:
              fprintf(listing, "%-11s", "Variable");
              fprintf(listing, "%-15s", "IntegerArray");
              break;
            default:
              break;
            }
            fprintf(listing, "\n");
            break;
          default:
            break;
          }
          bl = bl->next;
        }
      }
    }
    break;
  }
}

void print_FuncP_N_LoclVar(FILE *listing)
{
  int i, j;
  fprintf(listing, "\n< Function Parameters and Local Variables >\n");
  fprintf(listing, "  Scope Name    Nested Level     ID Name      Data Type \n");
  fprintf(listing, "--------------  ------------  -------------  -----------\n");

  for (i = 0; i < cntScope; i++)
  {
    ScopeList nowScope = scopes[i];
    if (strcmp(nowScope->funcName, "global") == 0)
      continue;
    BucketList *hashTable = nowScope->hashTable;
    //fprintf(listing,"%s\n",nowScope->funcName);

    int noParamVar = TRUE;
    for (j = 0; j < SIZE; j++)
    {
      if (hashTable[j] != NULL)
      {
        BucketList bl = hashTable[j];
        TreeNode *node = bl->treeNode;

        while (bl != NULL)
        {
          switch (node->nodekind)
          {
          case DeclK:
            noParamVar = FALSE;
            fprintf(listing, "%-16s", nowScope->funcName);
            fprintf(listing, "%-14d", nowScope->nestedLevel);
            switch (node->kind.decl)
            {
            case VarK:
              switch (node->type)
              {
              case Void:
                fprintf(listing, "%-15s", node->attr.name);
                fprintf(listing, "%-11s", "Void");
                break;
              case Integer:
                fprintf(listing, "%-15s", node->attr.name);
                fprintf(listing, "%-11s", "Integer");
                break;
              default:
                break;
              }
              break;
            case ArrVarK:
              fprintf(listing, "%-15s", node->attr.arr.name);
              fprintf(listing, "%-11s", "IntegerArray");
              break;
            default:
              break;
            }
            fprintf(listing, "\n");
            break;
          case ParamK:
            noParamVar = FALSE;
            fprintf(listing, "%-16s", nowScope->funcName);
            fprintf(listing, "%-14d", nowScope->nestedLevel);
            switch (node->kind.param)
            {
            case ArrParamK:
              fprintf(listing, "%-15s", node->attr.name);
              fprintf(listing, "%-11s", "IntegerArray");
              break;
            case NonArrParamK:
              fprintf(listing, "%-15s", node->attr.name);
              fprintf(listing, "%-11s", "Integer");
              break;
            default:
              break;
            }
            fprintf(listing, "\n");
            break;
          default:
            break;
          }
          bl = bl->next;
        }
      }
    }
    if (!noParamVar)
      fprintf(listing, "\n");
  }
}