/*
 *
 *  saxparser.h
 *
 *
 *   08/15/01
 */
#ifndef SAXPARSER_H
#define SAXPARSER_H

#ifndef SAXHANDLER_H
#include "saxhandler.h"
#endif

/* flags */
extern int namespaceEnabled;


extern char* getNamespacePrefix (char*);
extern char* getElementName (char*);
extern char* getValue (Attributes attribs, char* key);
extern char* getPITarget (char* s);
extern char* getPIData (char* s);
extern char* saveName (char* name);

#ifndef SAXPARSER_C

extern int nameTab;		/* 文字列テーブル */
extern int entityTab;		/* entityテーブル */
extern int attribTab;		/* element-attributeテーブル。エレメント名がキー */


#endif

#endif
