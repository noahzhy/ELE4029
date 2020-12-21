/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
static int location = 0;
static ScopeList globalScope = NULL;
static char *funcName;
static int inScopeBefore = FALSE;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse(TreeNode *t,
                     void (*preProc)(TreeNode *),
                     void (*postProc)(TreeNode *))
{
  if (t != NULL)
  {
    preProc(t);
    {
      int i;
      for (i = 0; i < MAXCHILDREN; i++)
        traverse(t->child[i], preProc, postProc);
    }
    postProc(t);
    traverse(t->sibling, preProc, postProc);
  }
}

static void typeError(TreeNode *t, char *message)
{
  fprintf(listing, "Error: Type error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

static void symbolError(TreeNode *t, char *message)
{
  fprintf(listing, "Error: Symbol error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

static void undeclaredError(TreeNode *t)
{
  if (t->kind.exp == CallK)
    fprintf(listing, "Error: Undeclared Function \"%s\" at line %d\n", t->attr.name, t->lineno);
  else if (t->kind.exp == IdK || t->kind.exp == ArrIdK)
    fprintf(listing, "Error: Undeclared Variable \"%s\" at line %d\n", t->attr.name, t->lineno);
  Error = TRUE;
}

static void redefinedError(TreeNode *t)
{
  if (t->kind.decl == FuncK)
    fprintf(listing, "Error: Redefined Function \"%s\" at line %d\n", t->attr.name, t->lineno);
  else if (t->kind.decl == VarK)
    fprintf(listing, "Error: Redefined Variable \"%s\" at line %d\n", t->attr.name, t->lineno);
  else if (t->kind.decl == ArrVarK)
    fprintf(listing, "Error: Redefined Variable \"%s\" at line %d\n", t->attr.arr.name, t->lineno);
  Error = TRUE;
}

static void funcDeclNotGlobal(TreeNode *t)
{
  fprintf(listing, "Error: Function Definition is not allowed at line %d (name : %s)\n", t->lineno, t->attr.name);
  Error = TRUE;
}

static void voidVarError(TreeNode *t, char *name)
{
  fprintf(listing, "Error: Variable Type cannot be Void at line %d (name : %s)\n", t->lineno, name);
  Error = TRUE;
}

/* input Function and output Function are
 * built-in Function, so there are global function
 * and have to print on Function Table 
*/
static void insertIOFuncNode(void)
{
  TreeNode *func;
  TreeNode *typeSpec;
  TreeNode *param;
  TreeNode *compStmt;

  func = newDeclNode(FuncK);
  typeSpec = newTypeNode(FuncK);
  typeSpec->attr.type = INT;
  func->type = Integer;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;
  compStmt->child[1] = NULL;

  func->lineno = 0;
  func->attr.name = "input";
  func->child[0] = typeSpec;
  func->child[1] = NULL;
  func->child[2] = compStmt;

  st_insert("input", 0, addLocation(), func);

  /* output Function */
  func = newDeclNode(FuncK);

  typeSpec = newTypeNode(FuncK);
  typeSpec->attr.type = VOID;
  func->type = Void;

  param = newParamNode(NonArrParamK);
  param->attr.name = "arg";
  param->type = Integer;
  param->child[0] = newTypeNode(FuncK);
  param->child[0]->attr.type = INT;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;
  compStmt->child[1] = NULL;

  func->lineno = 0;
  func->attr.name = "output";
  func->child[0] = typeSpec;
  func->child[1] = param;
  func->child[2] = compStmt;

  st_insert("output", 0, addLocation(), func);
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode *t)
{
  if (t == NULL)
    return;
  else
    return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      if (inScopeBefore)
        inScopeBefore = FALSE;
      else
      {
        ScopeList scope = sc_create(funcName);
        sc_push(scope);
        location++;
      }
      t->attr.scope = sc_top();
      break;
    default:
      break;
    }
    break;
  case ExpK:
    switch (t->kind.exp)
    {
    case IdK:
    case ArrIdK:
    case CallK:
      /* not yet in table, undeclared error */
      if (st_lookup(t->attr.name) == -1)
        undeclaredError(t);
      /* already in table, so ignore location, 
             add line number of use only */
      else
        st_add_lineno(t->attr.name, t->lineno);
      break;
    default:
      break;
    }
    break;
  case DeclK:
    switch (t->kind.decl)
    {
    case FuncK:
      funcName = t->attr.name;
      if (st_lookup_top(t->attr.name) >= 0)
      {
        redefinedError(t);
        break;
      }
      if (sc_top() != globalScope)
      {
        funcDeclNotGlobal(t);
        break;
      }
      st_insert(funcName, t->lineno, addLocation(), t);
      sc_push(sc_create(funcName));
      inScopeBefore = TRUE;

      switch (t->child[0]->attr.type)
      {
      case INT:
        t->type = Integer;
        break;
      case VOID:
      default:
        t->type = Void;
        break;
      }
      break;
    case VarK:
    case ArrVarK:
    {
      char *name;
      if (t->kind.decl == VarK)
      {
        name = t->attr.name;
        t->type = Integer;
      }
      else
      {
        name = t->attr.arr.name;
        t->type = IntegerArray;
      }

      if (st_lookup_top(name) < 0)
      {
        st_insert(name, t->lineno, addLocation(), t);
      }
      else
        redefinedError(t);
    }
    break;
    default:
      break;
    }
    break;
  case ParamK:
    if (t->child[0]->attr.type == VOID)
    {
      break;
    }

    if (st_lookup(t->attr.name) == -1)
    {
      st_insert(t->attr.name, t->lineno, addLocation(), t);
      if (t->kind.param == NonArrParamK)
        t->type = Integer;
      else
        t->type = IntegerArray;
    }
    break;
  default:
    break;
  }
}

static void afterInsertNode(TreeNode *t)
{
  if (t->nodekind == StmtK && t->kind.stmt == CompK)
    sc_pop();
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *syntaxTree)
{
  globalScope = sc_create("global");
  sc_push(globalScope);
  insertIOFuncNode();
  traverse(syntaxTree, insertNode, afterInsertNode);
  sc_pop();
}

static void beforeCheckNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case DeclK:
    switch (t->kind.decl)
    {
    case FuncK:
      funcName = t->attr.name;
      break;
    default:
      break;
    }
    break;
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      sc_push(t->attr.scope);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      sc_pop();
      break;
    case IfK:
    case IfEK:
      if (t->child[0] == NULL)
        typeError(t, "expected expression");
      else if (t->child[0]->type == Void)
        typeError(t->child[0], "invalid if condition type");
      break;
    case IterK:
      if (t->child[0] == NULL)
        typeError(t, "expected expression");
      else if (t->child[0]->type == Void)
        typeError(t->child[0], "invalid loop condition type");
      break;
    case RetK:
    {
      TreeNode *retFunc = get_bucket(funcName)->treeNode;
      if ((retFunc->type == Void && t->child[0] != NULL) ||
          (retFunc->type == Integer &&
           (t->child[0] == NULL || t->child[0]->type == Void || t->child[0]->type == IntegerArray)))
        typeError(t, "invalid return type");
      break;
    }
    default:
      break;
    }
    break;
  case ExpK:
    switch (t->kind.exp)
    {
    case AssignK:
      if (t->child[0]->type == Void || t->child[1]->type == Void)
        typeError(t->child[0], "invalid variable type");
      else if (t->child[0]->type == IntegerArray && t->child[0]->child[0] == NULL)
        typeError(t->child[0], "invalid variable type");
      else if (t->child[1]->type == IntegerArray && t->child[1]->child[0] == NULL)
        typeError(t->child[0], "invalid variable type");
      else
        t->type = t->child[0]->type;
      break;
    case OpK:
    {
      ExpType lType, rType;
      TokenType op;

      lType = t->child[0]->type;
      rType = t->child[1]->type;
      op = t->attr.op;

      if (lType == IntegerArray && t->child[0]->child[0] != NULL)
        lType = Integer;
      if (rType == IntegerArray && t->child[1]->child[0] != NULL)
        rType = Integer;

      if ((lType == Void || rType == Void) || (lType != rType))
        typeError(t, "invalid expression");
      else
        t->type = Integer;
      break;
    }
    case ConstK:
      t->type = Integer;
      break;
    case IdK:
    case ArrIdK:
    {
      BucketList l = get_bucket(t->attr.name);
      if (l == NULL)
        break;

      TreeNode *symbolNode = NULL;
      symbolNode = l->treeNode;

      if (t->kind.exp == ArrIdK)
      {
        if ((symbolNode->nodekind == DeclK && symbolNode->kind.decl != ArrVarK) || (symbolNode->nodekind == ParamK && symbolNode->kind.param != ArrParamK))
          typeError(t, "invalid expression");
        else
          t->type = symbolNode->type;
      }
      else
        t->type = symbolNode->type;
      break;
    }
    case CallK:
    {
      BucketList l = get_bucket(t->attr.name);
      TreeNode *funcNode = NULL;
      TreeNode *arg;
      TreeNode *param;

      if (l == NULL)
        break;
      funcNode = l->treeNode;
      arg = t->child[0];
      param = funcNode->child[1];

      if (funcNode->kind.decl != FuncK)
      {
        typeError(t, "invalid expression");
        break;
      }

      while (arg != NULL)
      {
        if (param == NULL || arg->type == Void)
        {
          typeError(arg, "invalid function call");
          break;
        }
        ExpType pType = param->type;
        ExpType aType = arg->type;
        if (aType == IntegerArray && arg->child[0] != NULL)
          aType = Integer;

        if (pType != aType)
        {
          typeError(arg, "invalid function call");
          break;
        }
        else
        {
          arg = arg->sibling;
          param = param->sibling;
        }
      }
      if (arg == NULL && param != NULL && param->child[0]->attr.type != VOID)
      {
        typeError(t->child[0], "invalid function call");
      }

      t->type = funcNode->type;
      break;
    }
    default:
      break;
    }
    break;
  case DeclK:
    switch (t->kind.decl)
    {
    case VarK:
    case ArrVarK:
      if (t->child[0]->attr.type == VOID)
      {
        char *name;
        if (t->kind.decl == VarK)
          name = t->attr.name;
        else
          name = t->attr.arr.name;
        voidVarError(t, name);
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree)
{
  sc_push(globalScope);
  traverse(syntaxTree, beforeCheckNode, checkNode);
  sc_pop();
}
