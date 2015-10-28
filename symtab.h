/*
 *
 *    symtab.h
 *
 *
 *
 *
 *  08/08/01
 */
#ifndef SYMTAB_H
#define SYMTAB_H

#ifndef Strtree_h
#include "strtree.h"
#endif



StrTreeNode getTree (int id);
void printKeys (int tabID);
int  newSymtab ();
void releaseSymtab (int tabID, void (*freeClosure)(void*));
void releaseSymtabAll (int tabID, void (*free_closure)(void*));

int lookupAndStoreData (int tabID, char* key, void* data);
void* lookupData (int tabID, char* key);
char* lookupKey (int tabID, char* key);
char* saveSymbolQuotesStripped (int tabID, char *s);
char* saveSymbol (int tabID, char *s);
char* saveSymbolAndData (int tabID, char *s, void* data);
char* saveSymbolAndString (int tabID, char *s, char* string);
char* saveKey (int tabID, char *s);
void insertData (int tabID, char *s, void* data);


#endif
