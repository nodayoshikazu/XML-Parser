/*
 *
 *    saxparser.c
 *
 *
 *  09/19/2001
 *
 */
#define SAXPARSER_C 1
#include <stdio.h>
#include <string.h>
#include "defaults.h"
#include "saxparser.h"
#include "saxhandler.h"
#include "symtab.h"
#include "attrib.h"
#include "mem.h"
#include "debug.h"

#ifdef WIN32
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#else
#define EXPORT /**/
#define IMPORT extern
#endif




#ifdef YYDEBUG
  extern int xxdebug;
#endif


/* namespaceを有効にするかどうか・
 */
int namespaceEnabled = 0;



/* table IDs */
int nameTab;			/* 文字列テーブル */
int entityTab;			/* entityテーブル */
int attribTab;			/* element-attributeテーブル。エレメント名がキー */

static char* predefEntities[] = {"amp", "apos", "gt", "lt", "quot"};
static char* predefEntValues[] =  {"&",   "'",    ">",  "<",  "\""};



/*
 * 基本的には開発時のバグ取り用
 */
void _fatal (char* m) 
{
  fprintf (stderr, "%s", m); 
  exit (1);
}

/*
 * あってはならないことではある。
 */
void _warning (char* m)
{
  fprintf (stderr, "%s", m);
}


/*
 * ENTITYテーブルを初期化する
 */

static void initEntityTab (int reftabID)
{
  int i;

  for (i = 0; i < sizeof(predefEntities)/sizeof(char*); i++)
    saveSymbolAndString (reftabID, predefEntities[i], predefEntValues[i]);
}

/*
 * Entityと対応する文字列をENTITYテーブルセーブする。
 */

void saveEntity (char* entity, char* value)
{
  saveSymbolAndString (entityTab, entity, value);
}

/*
 * string文字列の中のリファレンス(&...;)を対応する文字列に置き換えて
 *  destに返す。
 */

char* resolveReferences (char* string, char* dest)
{
  if (string && *string) {
    int i, j;
    char *q, *datap, *destp;
    char *s = string;
    char buf[MAX_REFERENCE_LENGTH];

    destp = dest;
    *destp = '\0';    
    while (*s) {
      if (*s == '&') {
	s++;			/* skip & */
	q = buf;
	while (*s && *s != ';')
	  *q++ = *s++;
	/* *s == ';' here */
	if (*s == '\0')
	  return NULL;		/* ERROR */
	s++;			/* skip ';' */
	*q = '\0';
	datap = (char*)lookupData (entityTab, buf);
	if (!datap) 
	  return NULL;		/* ERROR. entity is not defined for the string */
	strncpy (destp, datap, strlen (datap));
	destp += strlen (datap);
      } else {
	*destp++ = *s++;
      }
    }
    *destp = '\0';
  }
  return dest;
}

/*
 * nameテーブルに名前を登録する。
 */

char* saveName (char* name) 
{
  char* key;

  if ((key = lookupKey (nameTab, name)) == NULL)
    saveKey (nameTab, key = stringclone (name));
  return key;
}

/*
 * nameからnamespace名を取出す
 */

char* getNamespacePrefix (char* name)
{
  static char namespaceBuf[MAXNAMELEN];
  char* b = namespaceBuf;
  char* n = name;

  *b = '\0';			/* init */
  /* see if namespace prefix exists */
  while (*n && *n != ':')
    *b++ = *n++;
  if (*n == ':') {
    *b = '\0';
    return namespaceBuf;
  }
  return NULL;			/* no prefix at all */
}

/*
 * nameからnamespace名を除いた名前を取出す
 */

char* getElementName (char* name)
{
  static char elementnameBuf[MAXNAMELEN];
  char* b = elementnameBuf;
  char* n = name;

  *b = '\0';			/* init */
  /* see if namespace prefix exists */
  while (*n && *n != ':')
    n++;
  if (*n == ':')
    return ++n;
  return name;			/* no prefix at all */
}

/*
 * get PI target name
 */

char* getPITarget (char* s) 
{
  static char pitarget[MAXNAMELEN];
  char* t = pitarget;

  s++;				/* skip '<' */
  s++;				/* skip '?' */
  while (*s && !isspace (*s))
    *t++ = *s++;
  return pitarget;
}

/*
 * get DATA part of PI
 */

char* getPIData (char* s)
{
  static char pidata[MAXNAMELEN];
  char* t = pidata;

  s++;				/* skip '<' */
  s++;				/* skip '?' */
  while (*s && !isspace (*s))
    s++;
  while (*s && isspace (*s))
    s++;
  for (;*s;) {
    if (*s != '?')
      *t++ = *s++;
    else {
      if (*(s+1) == '>')
	break;
      else
	*t++ = *s++;
    }
  } 
  *t = '\0';
  return pidata;
}


/*
 * Parse XML file
 * returns 0 on success and nonzero on failure.
 */
extern char* lookupDTDPath (FILE*);

EXPORT int parse_xml (char* path)
{
  extern int nSymtabs;
  int r = 1;			/* nonzero=fail */
  FILE* file;
  char* dtdpath;

#ifdef YYDEBUG
  xxdebug = 0;			/* xml.y */
#endif
  nSymtabs = 0;			/* init number of symbol tables 09/19/01! */
  imem ();			/* initmem count */
  namespaceEnabled = 1;

  entityTab = newSymtab ();  /* リファレンステーブル */
  initEntityTab (entityTab); /* リファレンステーブル初期化 */
  nameTab = newSymtab ();	/* 名前テーブル */
  attribTab = newSymtab ();	/* エレメント属性テーブル */

  if (file = fopen (path, "r")) {
    if (dtdpath = lookupDTDPath (file)) {
      (void) parseDTD (dtdpath);
      DBGIN(printAllElementAttributes ();)	/* DEBUG */
      rewind (file);
      r = parseXml (file);
    }
    fclose (file);
  }
#ifdef DEBUG
  pmem ();			/* print memory size remaining un-freeed! */
  printKeys (nameTab);		/* debug */
#endif
  /* シンボルテーブルを解放する.  */
  releaseSymtab (entityTab, memfree);
  releaseSymtab (attribTab, releaseElementStruct);
  releaseSymtabAll (nameTab, memfree); /* nameはそこらじゅうで参照してるから最後に解放 */

  pmem ();			/* print memory size remaining un-freeed! */
  return r;
}
 
