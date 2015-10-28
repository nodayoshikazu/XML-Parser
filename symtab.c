/*
 *
 *    symtab.c
 *
 *
 *
 *   10/18/2001
 *
 */
#include <ctype.h>
#include "strtree.h"
#include "mem.h"
#include "debug.h"


/* シンボルテーブル(つりー)数 */
#define MAXSYMTABS  10

static StrTreeNode symtab[MAXSYMTABS] 
  = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int nSymtabs = 0;



/*
 */

StrTreeNode getTree (int id)
{
  if (id < nSymtabs)
    return symtab[id];
  __BUG ("getTree: arg ID is out of range");
}

/*
 * Print all keys
 */

void printKeys (int tabID) { 
  STreePrint (symtab[tabID]); 
}

/*
 * create a new symtab
 */

int newSymtab () {
  if (nSymtabs < MAXSYMTABS) {
    STreeInit (&symtab[nSymtabs]);
    nSymtabs++;
    return nSymtabs-1;
  }
  __BUG ("Too many symbol tables");
}

/*
 * release all symbols
 */

static void (*freeClosure)(void*);

static void releaseKeyAndClosure (StrTreeNode node)
{
  /*DBGIN(printf("releaseNode:Key=%s\n",Key(node));)*/
  if (isprint (*((char*)Key(node))))
    memfree ((char*) Key(node));
  if (Closure (node))
    (*freeClosure) (Closure (node));
}

static void releaseClosure (StrTreeNode node)
{
  if (Closure (node))
    (*freeClosure) (Closure (node));
}

/*
 */

void releaseSymtab (int tabID, void (*free_closure)(void*))
{
  if (symtab[tabID]) {
    freeClosure = free_closure;
    STreeDealloc (symtab[tabID], releaseClosure);
  }
}

/*
 */

void releaseSymtabAll (int tabID, void (*free_closure)(void*))
{
  if (symtab[tabID]) {
    freeClosure = free_closure;
    STreeDealloc (symtab[tabID], releaseKeyAndClosure);
  }
}


/*
 * store whatever into the closure
 */

int lookupAndStoreData (int tabID, char* key, void* data) {
  StrTreeNode node;

  if ((node = STreeSearchNode (symtab[tabID], key)) != NULL) {
    Closure (node) = data;
    return 0;
  }
  return -1;			/* node is not found */
}

/*
 * looksup the identifier and returns the closure to it.
 */

void* lookupData (int tabID, char* key)
{
  StrTreeNode node;

  if ((node = STreeSearchNode (symtab[tabID], key)) != NULL)
    return Closure (node);
  return NULL;
}

/*
 * lookup the identifier.
 */

char* lookupKey (int tabID, char* key)
{
  StrTreeNode node;

  if ((node = STreeSearchNode (symtab[tabID], key)) != NULL)
    return (char*) Key (node);
  return NULL;
}

/*
 * save a symbol as it is (maybe quoted?)
 */

char* saveKey (int tabID, char *s)
{
  if (lookupKey (tabID, s) == NULL)
    STreeInsert (&symtab[tabID], s, NULL);
  return s;
}

/*
 * save a symbol as it is (maybe quoted?)
 */

char* saveSymbolAndData (int tabID, char *s, void* data)
{
  if (lookupKey (tabID, s) == NULL)
    STreeInsert (&symtab[tabID], s, data);
  return s;
}

/*
 * save a symbol as it is (maybe quoted?)
 */

void insertData (int tabID, char *s, void* data)
{
  STreeInsert (&symtab[tabID], s, data);
}

/*
 * save a symbol as it is (maybe quoted?)
 */

char* saveSymbolAndString (int tabID, char *s, char* string)
{
  if (lookupKey (tabID, s) == NULL)
    STreeInsert (&symtab[tabID], s, stringclone (string));
  return s;
}


