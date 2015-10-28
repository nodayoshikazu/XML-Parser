%{
/*
 *   xml.l
 *
 *
 *  note:
 *   WIN32 stuff. These include lines should be manually removed from code for Win32 compilation.
 * #include <inttypes.h>
 * #include <widec.h>
 * #include <sys/euc.h>
 *
 *   11/16/01
 */
#define _POSIX_SOURCE 1
#ifndef WIN32
#define __STDC__ 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef unix
#include <unistd.h>
#include <strings.h>
#endif

#ifdef JAVA
#include <jni.h>
#endif
#include "xml.tab.h"
#include "symtab.h"
#include "debug.h"


extern void warn(char*);  /* see xml.y */

int LINE_NO;

#undef input
#ifdef WIN32
/* win-nt doesn't have inttype.h, so */
typedef unsigned int * uintptr_t;
 int euclen (char* s) { return 1; }
#endif

#ifndef __cplusplus
#undef input()
#define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):(*nextc)(lexsrc))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)

#else
#undef lex_input
#define lex_input() \
(((yytchar=yysptr>yysbuf?U(*--yysptr):(*nextc)(lexsrc))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif

static void* lexsrc; /* input source..buffer top or FILE* */
static int   memptr; /* buffer index when inp src is buffer top */
static int   memlen; /* buffer size when inp src is buffer top */
static int   (*nextc)(void*); /* pointer to input function */

/* input source is a null terminated string on memory */
static int _getc_memory_(void* mem) {
  unsigned char c = ((unsigned char*)mem)[memptr++];
  return (c == '\0') ? EOF : c;
}

/* input source is a byte array with known length */
static int _getc_memory_len_(void* mem) {
  return (memptr >= memlen) ? EOF : ((unsigned char*)mem)[memptr++];
}

/* input source is a file */
static int _getc_file_ (void *f) {
  return (fgetc ((FILE*)f));
}

static int charnext;
static char charbuf[1024];
static char resultbuf[1024];
static char msgbuf[256];

void end_chardata();


/*
 */
static void xmlInitLineno ()
{
  extern int LINE_NO;
  LINE_NO = 1;
}

/*
 */
static void fileParsingMode (FILE* file) {
  lexsrc = file;
  nextc = _getc_file_;
}

/*
 */
static void memoryParsingMode (char* buffer, int length) {
  memptr = 0;
  memlen = length;
  lexsrc = (void*)buffer;
  nextc = _getc_memory_len_;
}


/*
 * Parses file
 */

int parseXml (FILE* file)
{
  int r;

  BEGIN 0;
  xmlInitLineno();		/* 10/28/01 */
  lexsrc = file;
  nextc = _getc_file_;
  r = yyparse ();		/* 10/28/01 */
  return r;
}

/*
 * Parses buffer
 */

int parseXmlMemory (char* buffer, int len) 
{
  int r;
  void* srcsave;

  BEGIN 0;
  xmlInitLineno();		/* 10/28/01 */
  memptr = 0;
  memlen = len;
  srcsave = lexsrc;
  lexsrc = (void*)buffer;
  nextc = _getc_memory_len_;
  r = yyparse ();		/* 10/28/01 */
  /* back to file */
  lexsrc = srcsave;
  nextc = _getc_file_;
  return r;
}

/*
 * まだリファレンスの取出しがはいっていない
 */

static int charCollecter () {
  /*printf ("....%02x\n", yytext[0]);*/
  if (yytext[0] & 0x80) {
    /* 漢字 */
    charbuf[charnext++] = yytext[0];
    charbuf[charnext++] = yytext[1];
  } else {
    if (yytext[0] == '<') {
      yyunput (yytext[0]);
      charbuf[charnext] = '\0'; /* terminator */
      yylval.string = charbuf; 
      end_chardata();		/* BEGIN xml */
      return 1;
    }
    else
      charbuf[charnext++] = yytext[0];
    if (yytext[0] == '\n')
      LINE_NO++;
  }
  return 0;  /* not yet */
}

/*
 * まだリファレンスの取出しがはいっていない
 */
static int charCollecter2 (int quote) {
  if (yytext[0] & 0x80) {
    /* 漢字 */
    charbuf[charnext++] = yytext[0];
    charbuf[charnext++] = yytext[1];
  } else {
    if (yytext[0] == quote) {
      yyunput (yytext[0]);
      charbuf[charnext] = '\0'; /* terminator */
      yylval.string = charbuf; 
      end_chardata();		/* BEGIN xml; */
      return 1;
    }
    else if (yytext[0] == '\n')
      LINE_NO++;
    else
      charbuf[charnext++] = yytext[0];
  }
  return 0;  /* not yet */
}
/*static int charCollecter (), charCollecter2 (int);*/

extern int nameTab;
extern char* resolveReferences (char* string, char* dest); /* returns dest */

/*
anychar         ([\001-\177]|[\200-\376].)
dquotedstring	\"([^\"]|[\200-\376].)*\"
squotedstring	\'([^\']|[\200-\376].)*\'
*/

%}

letter		[A-Za-z]
digit		[0-9]
kanji           [\200-\376].
name		([A-Za-z]|_|:)([A-Za-z]|[0-9]|\.|\-|_|:)*
white           [ \t\r]+
charref		("&#"[0-9]+)|("&#x"[0-9a-fA-F]+)";"
entityref	"&"([A-Za-z]|"_"|":")([A-Za-z]|[0-9]|"."|"-"|"_"|":")*";"
pentityref	"%"([A-Za-z]|"_"|":")([A-Za-z]|[0-9]|"."|"-"|"_"|":")*";"
comment		"<!--""--"*([^\->]|[^\-]">"|"-"[^>])*"--"*"-->"
versionnum	\"[a-zA-Z0-9\.:\-]+\"
versionnum2	"'"[a-zA-Z0-9\.:\-]+"'"
pi  "<?"([A-Za-z]|_|:)([A-Za-z]|[0-9]|\.|\-|_|:)*[ \t\r\n]+"?"*([^\?>]|[^\?]">"|"?"[^>])*">"*"?>"
cdata_begin	"<![CDATA["
cdata_rest	"]]"*([\]\]]|[^\]]"]>"|"]"[^>])*">"*"]]>"
chardata	[^&<]+
anychar         ([\001-\177]|[\200-\376].)
CCcomment	"/*" "/"* ( [^*/] | [^*]"/" | "*"[^/] )* "*"* "*/"


%start xml CData CharData singleq doubleq
%p 5000
%a 10000
%o 20000

%%

"<?xml"			{ BEGIN xml; return LT_Q_XML; }
<xml>"EMPTY"		{ return EMPTY; }
<xml>"ANY"		{ return ANY; }
<xml>"CDATA"		{ return CDATA; }
<xml>"ID"		{ return ID; }
<xml>"IDREF"		{ return IDREF; }
<xml>"IDREFS"		{ return IDREFS; }
<xml>"IGNORE"		{ return _IGNORE; }
<xml>"INCLUDE"		{ return INCLUDE; }
<xml>"ENTITY"		{ return ENTITY; }
<xml>"ENTITIES"		{ return ENTITIES; }
<xml>"NMTOKEN"		{ return NMTOKEN; }
<xml>"NMTOKENS"		{ return NMTOKENS; }
<xml>"NOTATION"		{ return NOTATION; }
<xml>"SYSTEM"		{ return SYSTEM; }
<xml>"PUBLIC"		{ return PUBLIC; }
<xml>"NDATA"		{ return NDATA; }
<xml>"encoding"		{ return ENCODING; }
<xml>"standalone"	{ return STANDALONE; }
<xml>"version"		{ return VERSION; }
<xml>"&amp"		{ return REF_AMP; }
<xml>"&apos"		{ return REF_APOS; }
<xml>"&gt"		{ return REF_GT; }
<xml>"&lt"		{ return REF_LT; }
<xml>"&quot"		{ return REF_QUOT; }
<xml>"#PCDATA"		{ return S_PCDATA; }
<xml>"#REQUIRED"	{ return S_REQUIRED; }
<xml>"#IMPLIED"		{ return S_IMPLIED; }
<xml>"#FIXED"		{ return S_FIXED; }
<xml>"<!ATTLIST"	{ return LT_EX_ATTLIST; }
<xml>"<!DOCTYPE"	{ return LT_EX_DOCTYPE; }
<xml>"<!ELEMENT"	{ initDfa (); return LT_EX_ELEMENT; }
<xml>"<!ENTITY"		{ return LT_EX_ENTITY; }
<xml>"<!NOTATION"	{ return LT_EX_NOTATION; }
<xml>"\"yes\""		{ return QYES; }
<xml>"\'yes\'"		{ return AYES; }
<xml>"\"no\""		{ return QNO; }
<xml>"\'no\'"		{ return ANO; }
<xml>"="		{ return '='; }

<xml>{pi}		{ yylval.string = yytext; return PI; }

<xml>"<![CDATA["	{ BEGIN CData; return LT_EX_SB_CDATA; }
<CData>{cdata_rest}	{ BEGIN xml; return CDATA_REST; }

<xml>{name}		{ extern char* saveName (char*);
                          char* s = saveName (yytext);
			  yylval.string = s; return NAME; }
<xml>{charref}		{ return CHARREF; }
<xml>{entityref}	{ return EREF; }
<xml>{pentityref}	{ return PEREF; }
<xml>{comment}		{ /* returns NO TOKEN*/ }

<xml>"<!["		{ return LT_EX_SB; }
<xml>"<?"		{ return LT_Q; }
<xml>"</"		{ return LT_SLASH; }
<xml>"]]>"		{ return SB_SB_GT; }
<xml>"/>"		{ return SLASH_GT; }
<xml>">"		{ return '>'; }
<xml>"<"		{ return '<'; }
<xml>"["		{ return '['; }
<xml>"]"		{ return ']'; }

<xml>";"		{ return ';'; }
<xml>","		{ return ','; }
<xml>"("		{ return '('; }
<xml>")"		{ return ')'; }
<xml>"\""		{ return DQUOTE; }	
<xml>"'"		{ return SQUOTE; }
<xml>"%"		{ return '%'; }
<xml>"&"		{ return '&'; }
<xml>"|"		{ return '|'; }
<xml>"*"		{ return '*'; }
<xml>"+"		{ return '+'; }
<xml>"-"		{ return '-'; }
<xml>"?"		{ return '?'; }

<singleq>{anychar}	{ 
                          if (charCollecter2 ('\'')) {
			    /* yylval.string = resultbuf */
			    yylval.string = resolveReferences (charbuf, resultbuf);
			    if (yylval.string == NULL) {
			      sprintf (msgbuf, "Entity is not defined for %s", charbuf);
			      warn (msgbuf);
			    }
			    return SQUOTEDSTRING;
			  }
                        }
<doubleq>{anychar}	{ 
                          if (charCollecter2 ('\"')) {
			    /* yylval.string = resultbuf */
			    yylval.string = resolveReferences (charbuf, resultbuf);
			    if (yylval.string == NULL) {
			      sprintf (msgbuf, "Entity is not defined for %s", charbuf);
			      warn (msgbuf);
			    }
			    return DQUOTEDSTRING; 
			  }
                        }
<CharData>{anychar}	{ 
			  /* 文字データが０個の場合もここでCHARDATAトークンが返される
			   * いきなり'<'が現れる場合もあり
			   */
                          if (charCollecter ()) {
			    /* yylval.string = resultbuf */
			    yylval.string = resolveReferences (charbuf, resultbuf);
			    if (yylval.string == NULL) {
			      sprintf (msgbuf, "Entity is not defined for %s", charbuf);
			      warn (msgbuf);
			    }
			    return CHARDATA;
			  }
                        }
<xml>{white}		;
<xml>"\n"		{ LINE_NO++; }
<xml>.			;


%%

void begin_chardata () { BEGIN CharData; charnext = 0; charbuf[0] = resultbuf[0] = '\0'; }
void begin_doubleq () { BEGIN doubleq; charnext = 0; charbuf[0] = resultbuf[0] = '\0'; }
void begin_singleq () { BEGIN singleq; charnext = 0; charbuf[0] = resultbuf[0] = '\0'; }
void end_chardata () { BEGIN xml; }
void begin_cdata () { BEGIN CData; }	

int yywrap() {  return 1; }



/*
 * END
 */

#if 0
void main (int ac, char* av[]) {
  extern int yydebug;
  int token;

  yydebug = 1;
  while (token = yylex())
    printf("line=%d [token=%d] text=%s\n",LINE_NO, token, yytext );
}
#endif


