%{
/************************************************************
 *
 *  xml.y
 *  
 *  DTDパース用の文法も含みます。
 *  一点だけ、DTDで不可能な書き方があります。w3cドラフトでは以下のようにXMLバージョン情報が
 *    XMLヘッダーに含めますが、ここにある文法では、それを許していません。
 *
 * extSubset
 *	: TextDecl extSubsetDecl
 *	| extSubsetDecl
 *	;
 * TextDecl
 *	: "<?xml" VersionInfo EncodingDecl '>'
 *	| "<?xml" EncodingDecl '>'
 *	;
 *
 *  note:
 *   WIN32 stuff. These include lines should be manually removed from code for Win32 compilation.
 * #include <inttypes.h>
 * #include <values.h>
 *
 *
 *   06/28/01
 *   06/29/01
 *   07/03/01
 *   07/09/01
 *   07/13/01
 *   07/23/01  namespace processing added
 *   08/06/01  changed yyerror() to saywhy()
 *   08/10/01
 *   08/14/01  bug fixes and error checkings for attribute values
 *   08/15/01
 *   08/21/01  sax_startDocument, sax_endDocument added
 ************************************************************/
#define _POSIX_C_SOURCE 1
#define _POSIX_SOURCE 1
#define __EXTENSIONS__ 1	/* to get uint */


#ifndef unix
#define MSDOS 1			/* for gnu alloca */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef unix
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
/*#include <inttypes.h>*/
#ifndef __GNUC__
#include <alloca.h>
#endif /*__GNUC__*/
#else 
/*WIN32*/
#include <windows.h>
#endif /*unix*/
#ifdef JAVA
#include <jni.h>
#endif
#include "defaults.h"
#include "strtree.h"
#include "saxparser.h"
#include "saxhandler.h"
#include "mem.h"
#include "dfa.h"
#include "attrib.h"
#include "stack.h"
#include "debug.h"





/* forward decl */
static void yyerror (char*);
static void _error (char* msg);
/*public*/ void warn(char*);
/*public*/ void warn2(char*, char*);
/*public*/ void warn3(char*, char*, char*);


/* namespace URI stack */
static XStack* xstack_xmlns_uri;

/* element stack */
static XStack* xstack_element;

static char* xmlns = "xmlns";
static char  lastXmlnsURI[URI_LENGTH];	/* namespace URI */
static int   xmlnsValid = FALSE;

static Element*   current_element; /* Element* */
static Element*   parent_element; /* Element* */
static Attribute* current_attribute; /* Attribute* */
static unsigned   current_eltype; /* element type */

static char*      namesbuf[32];

#define OHBOY  onLeavingParser();return(1);


/* アトリビュートリスト用ツリールート */
static StrTreeNode attribTreeRoot = NULL;

/*
 * attributeを解放
 */

static void _releaseAttribute (StrTreeNode node)
{
  /*
   * keyはnameTreeにつながっているのでフリーしない。
   */
  if (Closure (node))
    memfree ((char*)Closure (node));
}

/*
 * attributeを解放
 */

static void releaseAttributes ()
{
  if (attribTreeRoot) {
    STreeDealloc (attribTreeRoot, _releaseAttribute);
    attribTreeRoot = NULL;
  }
}

/* 
 * attributeを追加
 */
static void addAttribute (char* name, char* value)
{
  if (attribTreeRoot == NULL)
    STreeInit (&attribTreeRoot); /* 初期化 */
  /*
   * valueはまだメモリーに割り付けられていない
   */
  if (value && *value) {
    if ((STreeSearchNode (attribTreeRoot, name)) == NULL) {
      char* vp = stringclone (value);
      STreeInsert (&attribTreeRoot, name, vp);
    }
  }
}

/*
 *  パーサ初期化
 */
static void onEnteringParser (void)
{
  xmlnsValid = FALSE;
  xstack_xmlns_uri = newXStack ();
  xstack_element = newXStack ();
}

/* 
 * パーサ後掃除
 */
static void onLeavingParser (void) 
{
  releaseAttributes ();
  releaseXStack (xstack_xmlns_uri);
  releaseXStack (xstack_element);
}


%}

%union {
  int    integer;
  char*  string;
  void*  voidptr;
}

%token EMPTY
%token ANY
%token CDATA
%token ID
%token IDREF
%token IDREFS
%token ENTITY
%token ENTITIES
%token NMTOKEN
%token NMTOKENS
%token NOTATION
%token ENCODING
%token STANDALONE
%token SYSTEM
%token PUBLIC
%token NDATA
%token VERSION
%token REF_AMP
%token REF_APOS
%token REF_GT
%token REF_LT
%token REF_QUOT
%token S_PCDATA
%token S_REQUIRED
%token S_IMPLIED
%token S_FIXED
%token LT_EX_ATTLIST
%token LT_EX_DOCTYPE
%token LT_EX_ELEMENT
%token LT_EX_ENTITY
%token LT_EX_NOTATION
%token LT_EX_SB_CDATA
%token INCLUDE
%token _IGNORE
%token LT_Q_XML
%token CHARREF
%token NAME
%token EREF
%token PEREF
%token WHITE
%token LT_EX_SB
%token LT_Q
%token LT_SLASH
%token SB_SB_GT
%token SLASH_GT
%token QYES
%token AYES
%token QNO
%token ANO
%token DQUOTE
%token SQUOTE
%token DQUOTEDSTRING
%token SQUOTEDSTRING
%token PI
%token CDATASECT
%token CDATA_REST
%token VERSIONNUM
%token CHARDATA
%token KANJI
%token ';' ',' '(' ')' '=' '%' '&' '|' '*' '+' '-' '>' '<' '?'

%type <string> DQUOTEDSTRING SQUOTEDSTRING QuotedString
%type <string> NAME CHARDATA PI
%type <string> STag ETag EmptyElemTag
%type <string> ExternalID SystemLiteral PublicLiteral EntityDef EntityValue PEDef
%type <integer> AttType DefaultDecl
%type <voidptr> exp cp sequences alternations contentspec

%%

/*
 * XML grammar
 */

xml
	:
	  { 
	    onEnteringParser ();
	    (*sax_startDocument)();
	  }
	document
	  {
	    (*sax_endDocument)();
	    onLeavingParser ();
	  }
	;

document 
	: prolog element miscelanious
	| prolog element 
	| DTDStart somemarkupdecls      { /* compatibility with DTD syntax */ }
	| error				{ _error ("wrong xml header."); OHBOY; }
	;

prolog
	: XMLDecl miscelanious doctypedecl
	| XMLDecl doctypedecl miscelanious
	| XMLDecl doctypedecl 
	| XMLDecl 
	;

miscelanious
	: misc
	| miscelanious misc
	| error			{ _error ("Only PIs are allowed here."); OHBOY; }
	;

misc
	: PI
	  {
	    (*sax_processingInstruction)(getPITarget ($1), getPIData ($1));
	  }
	;

doctypedecl
	: LT_EX_DOCTYPE NAME ExternalID '[' markupdecls ']' '>'
	| LT_EX_DOCTYPE NAME ExternalID '>'
	| LT_EX_DOCTYPE NAME '[' markupdecls ']' '>'
	| LT_EX_DOCTYPE NAME ExternalID '[' error { _error("wrong DOCTYPE declaration."); OHBOY; }
	| LT_EX_DOCTYPE NAME ExternalID error	{ _error("wrong DOCTYPE declaration."); OHBOY; }
	| LT_EX_DOCTYPE NAME error		{ _error("wrong DOCTYPE declaration."); OHBOY; }
	| LT_EX_DOCTYPE error			{ _error("wrong DOCTYPE declaration."); OHBOY; }
	;

XMLDecl
	: LT_Q_XML VersionInfo EncodingDecl SDDecl '?' '>'
	| LT_Q_XML VersionInfo EncodingDecl '?' '>'
	| LT_Q_XML VersionInfo SDDecl '?' '>'
	| LT_Q_XML VersionInfo '?' '>'
	| LT_Q_XML VersionInfo EncodingDecl SDDecl error { _error("wrong xml header."); OHBOY; }
	| LT_Q_XML VersionInfo EncodingDecl error	{ _error("wrong xml header."); OHBOY; }
	| LT_Q_XML VersionInfo error			{ _error("wrong xml header."); OHBOY; }
	| LT_Q_XML error				{ _error("wrong xml header."); OHBOY; }
	;

DTDStart
	: LT_Q_XML EncodingDecl '?' '>'			{ /*DTD compatibility*/ }
	;

QuotedString
	: DQUOTE { begin_doubleq(); } DQUOTEDSTRING DQUOTE { $$ = $3; }
	| SQUOTE { begin_singleq(); } SQUOTEDSTRING SQUOTE { $$ = $3; }
	;

VersionInfo
	: VERSION '=' QuotedString	{ /* string my not contain references 
					   * bu may contain illegal chars.
					   *  	\"[a-zA-Z0-9\.:\-]+\"
					   */ }
	;

EncodingDecl
	: ENCODING '=' QuotedString
	| ENCODING '=' error		{ _error("must specify character set here."); OHBOY; }
	| ENCODING error		{ _error("must specify character set here."); OHBOY; }
	;

/* standalone=yes|no */
SDDecl
	: STANDALONE '=' YesOrNo
	| STANDALONE '=' error		{ _error("must specify yes or no."); OHBOY; }
	| STANDALONE error		{ _error("must specify yes or no."); OHBOY; }
	;

YesOrNo
	: QYES | QNO | AYES | ANO
	;

element
	: STag 
	  {
	    char* nsuri = NULL;	/* namespace uri */
	    char* elem = NULL;	/* element name */
	    int   i, error;

	    /*
	     * 予測外のエレメントがあったら警告???でいいのか???
	     */
	    if (parent_element = xtop (xstack_element))
	      if (getNextStateOnSymbol (parent_element, $1) == NOT_ACCEPT) {
		warn2 ("Encountered illegal element...", $1);
		/*************************************************
		 * とりあえずチェックだけという場合もあるからコンテキストエラーでは
		 * アボートはしないでおく。
		 *************************************************/
		/*OHBOY; abort */
	      }
	    /* このエレメント名のデータ属性を得る */
	    current_element = (Element*) lookupData (attribTab, $1);
	    initStateWorkOfElement (current_element);
	    xpush (xstack_element, current_element);

            /* #REQUIREDなのに存在しないアトリビュートがあるか調べる
	     *   あれば警告表示 ??? でいいのか???
	     */
            error = checkAttributesForRequired (current_element, attribTreeRoot, namesbuf);
	    if (0 < error) {
	      for (i = 0; i < error; i++)
		warn2 ("Missing attribute...", namesbuf[i]);
	      /*************************************************
	       * とりあえずチェックだけという場合もあるからコンテキストエラーでは
	       * アボートはしないでおく。
	       *************************************************/
	      /*OHBOY; abort */
	    }
            /* enum文字列の妥当性チェック */
            error = checkAttributesForFixed (current_element, attribTreeRoot, namesbuf);
            if (0 < error) {
	      for (i = 0; i < error; i++)
		warn3 (namesbuf[i*2+1], " is illegal enumeration item for ", namesbuf[i*2]);
	    }

	    if (namespaceEnabled) {
	      nsuri = xtop (xstack_xmlns_uri);
	      elem = getElementName ($1);
	    }
            (*sax_startElement)(nsuri, elem, $1, attribTreeRoot);
	    releaseAttributes ();
	  } 
	contents 
	ETag 
	  { 
	    char* nsuri = NULL;
	    char* elem = NULL;

	    /* 親エレメント属性をポップ */
	    (void) xpop (xstack_element);

	    if (namespaceEnabled) {
	      elem = getElementName ($4);
	      nsuri = xtop (xstack_xmlns_uri);
	    }
	    (*sax_endElement)(nsuri, elem, $4);
	    releaseAttributes ();
	  }
	| EmptyElemTag 
	  { 
	    if (parent_element = xtop (xstack_element))
	      if (getNextStateOnSymbol (parent_element, $1) == NOT_ACCEPT)
		warn2 ("Encountered illegal element...", $1);

	    /* このエレメント名のデータ属性を得る */
	    if (current_element = (Element*) lookupData (attribTab, $1)) {
	      initStateWorkOfElement (current_element); /* anyways */

	      if (TypeOfElement (current_element) != EL_EMPTY) {
		warn2 ($1, " cannot be an empty element");
		/*************************************************
		 * とりあえずチェックだけという場合もあるからコンテキストエラーでは
		 * アボートはしないでおく。
		 *************************************************/
		/*OHBOY;*//* abort */
	       }
	    }
	    if (namespaceEnabled) {
	      char* uri = xtop (xstack_xmlns_uri);
	      (*sax_startElement)(uri, getElementName ($1), $1, attribTreeRoot);
	      (*sax_endElement)(uri, getElementName ($1), $1);
	    }
	    else {
	      (*sax_startElement)(NULL, NULL, $1, attribTreeRoot);
	      (*sax_endElement)(NULL, NULL, $1);
	    }
	    releaseAttributes ();
	  }
	;

EmptyElemTag
	: '<' NAME Attributes SLASH_GT 
	  {
	    $$ = $2;   /* NAME */
	  }
	| '<' NAME SLASH_GT 
	  {
	    $$ = $2;   /* NAME */
	  }
	;

contents
	: content
	| contents content	
	| error				{ _error("illegal xml content."); OHBOY; }
	;

content
	: CHARDATA
	  {
	    /*
	     * インデントの空白やタブもここに入ってきてしまうので、このチェックは使えない。
	     * ま、このチェックはいらないようだ。
	     * if (current_element)
	     * if (TypeOfElement (current_element) != EL_PCDATA)
	     * warn2 (NameOfElement (current_element), " cannot be a #PCDATA ");
	     */
	    int len = 0;
	    if ($1) 
	      len = strlen ($1);
	    (*sax_characters)($1, 0, len);
	  }
	| Reference	{ begin_chardata (); }
	| CDSect	{ begin_chardata (); }
	| PI		
	  {
	    (*sax_processingInstruction)(getPITarget ($1), getPIData ($1));
	    begin_chardata ();
	  }
	| element
	;

Reference
	: CHARREF	{ /* &#ddd; CharRef */ }
	| EREF		{ /* &xxx;  Entity Reference */ }
	| REF_AMP	{ /* &amp;  replace with & */ }
	| REF_LT	{ /* &lt;   replace with < */ }
	| REF_GT	{ /* &gt;   replace with > */ }
	| REF_QUOT	{ /* &quot; replace with " */ }
	| REF_APOS	{ /* &apos; replace with ' */ }
	;

STag
	: '<' NAME Attributes '>' 
	  {
	    if (namespaceEnabled) {
	      char* nsuri = lastXmlnsURI;

	      if (xmlnsValid && *nsuri) {
		nsuri = saveName (nsuri);
		xpush (xstack_xmlns_uri, nsuri);
              }
	      else
		xpush (xstack_xmlns_uri, NULL);
	    }
	    if (namespaceEnabled) {		/* namespaceが有効 */
	      if (xmlnsValid)
		(*sax_startPrefixMapping)(getNamespacePrefix ($2), lastXmlnsURI);
	      xmlnsValid = FALSE;
	      lastXmlnsURI[0] = '\0';
	    }
	    begin_chardata (); 
	    $$ = $2;   /* NAME */
	  }
	| '<' NAME '>' 
	  {
	    if (namespaceEnabled) 
	      xpush (xstack_xmlns_uri, NULL); /* dummy */
	    begin_chardata (); 
	    $$ = $2;   /* NAME */
	  }
	| '<' NAME Attributes error	{ _error("missing closing '>'."); OHBOY; }
	;

ETag
	: LT_SLASH NAME '>' 
	  {
	    char* uri = NULL;

	    if (namespaceEnabled) {
	      uri = xpop (xstack_xmlns_uri);
	      if (uri)
		(*sax_endPrefixMapping)(getNamespacePrefix ($2));
	    }
	    $$ = $2;  /* NAME */
	  }
	| LT_SLASH NAME error		{ _error("missing closing '>'."); OHBOY; }
	| LT_SLASH error		{ _error("missing tag name and closing '>'."); OHBOY; }
	;

/* CDATA section */
CDSect
	: LT_EX_SB_CDATA CDATA_REST
	;

Attributes
	: Attributes Attribute
	| Attribute
	| error				{ _error("illegal attribute."); OHBOY; }
	;

/*
 * この文字列の中にはリファレンス(Reference|PEReference)がふくまれてよい。
 *  したがってプログラム的に、リファレンスを変換する必要がある。
 */
Attribute
	: NAME '=' QuotedString 
	  {    /* string my contain references */ 
	    if (strncmp (xmlns, $1, 5) == 0) {
	      strcpy (lastXmlnsURI, $3); /* save last xmlns URI */
	      xmlnsValid = TRUE;
	    }
	    addAttribute ($1, $3);
	    DBGIN(if ($3!=NULL)
                    printf("addAttribute called for %s=%s\n", $1, $3);
	              else
		    printf("addAttribute called for %s=NULL\n", $1);
	    )
	  }
	| NAME '=' error		{ _error("illegal attribute value."); OHBOY; }
	| NAME error			{ _error("illegal attribute value."); OHBOY; }
	;

ExternalID
	: SYSTEM SystemLiteral			{ $$ = $2; }
	| PUBLIC PublicLiteral SystemLiteral	{ $$ = $2; }
	| SYSTEM error			{ _error("illegal uri spec."); OHBOY; }
	| PUBLIC PublicLiteral error	{ _error("illegal uri spec."); OHBOY; }
	| PUBLIC error			{ _error("illegal uri spec."); OHBOY; }
	;

SystemLiteral
	: QuotedString		{ $$ = $1; }
	;

PublicLiteral
	: QuotedString		{ $$ = $1; }
	;

/* 
 * DTD要素解析の文法
 */
markupdecls
	: markupdecls markupdecl
	| markupdecl
	/*| error*/
	;

markupdecl
	: elementdecl
	| AttlistDecl
	| EntityDecl 
	| NotationDecl
	| PI
	  {
	    (*sax_processingInstruction)(getPITarget ($1), getPIData ($1));
	  }
	;
	
/*
 * <!ELEMENT Name contentspec>
 */
elementdecl
	: LT_EX_ELEMENT NAME contentspec '>'
          {
	    if ($2 == NULL) 
	      warn ("elementdecl: NAME was NULL");
	    else {
	      if ((current_element = (Element*) lookupData (attribTab, $2)) == NULL) {
		current_element = newElementStruct ($2);
		insertData (attribTab, $2, current_element);
	      }
	      if ($3 != NULL)
		setDfaToElement (current_element, $3);
	      else
		setTypeToElement (current_element, current_eltype);
	    }
	  }
	| LT_EX_ELEMENT NAME contentspec error	{ _error("illegal ELEMENT spec. Missing '>'."); OHBOY; }
	| LT_EX_ELEMENT NAME error		{ _error("illegal ELEMENT spec."); OHBOY; }
	| LT_EX_ELEMENT error			{ _error("illegal ELEMENT spec. Missing ELEMENT name."); OHBOY; }
	;

contentspec
	: EMPTY
	  {
	    $$ = NULL;
	    current_eltype = EL_EMPTY;
	  }
	| ANY
	  {
	    $$ = NULL;
	    current_eltype = EL_ANY;
	  }
	| '(' alternations ')'
	  {
	    $<voidptr>$ = $2;
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' alternations ')' '?'
	  {
	    /* exp? = exp|empty */
	    $<voidptr>$ = makeNode (RE_OR, $<voidptr>2, emptyNode);
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' alternations ')' '*'
	  {
	    $<voidptr>$ = makeNode (RE_STAR, $<voidptr>2, NULL);
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' alternations ')' '+'
	  {
	    /* r+ = rr* */
	    $<voidptr>$ = makeNode (RE_CAT, $<voidptr>2, makeCopyNode (RE_STAR, $<voidptr>2, NULL));
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' sequences ')'
	  {
	    $<voidptr>$ = $2;
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' sequences ')' '?'
	  {
	    /* exp? = exp|empty */
	    $<voidptr>$ = makeNode (RE_OR, $<voidptr>2, emptyNode);
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' sequences ')' '*'
	  {
	    $<voidptr>$ = makeNode (RE_STAR, $<voidptr>2, NULL);
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' sequences ')' '+'
	  {
	    /* r+ = rr* */
	    $<voidptr>$ = makeNode (RE_CAT, $<voidptr>2, makeCopyNode (RE_STAR, $<voidptr>2, NULL));
	    $$ = createDFA ($<voidptr>$);
	    current_eltype = EL_DFA;
	  }
	| '(' S_PCDATA pcd_follows ')' '*'   /* Mixed */
          {
	    $$ = NULL;
	    current_eltype = EL_PCDATA;
	  }
	| '(' S_PCDATA pcd_follows ')'       /* Mixed */
          {
	    $$ = NULL;
	    current_eltype = EL_PCDATA;
	  }
	| '(' S_PCDATA ')'               /* Mixed */
          {
	    $$ = NULL;
	    current_eltype = EL_PCDATA;
	  }
	| '(' error			{ _error("illegal ELEMENT spec."); OHBOY; }
	;

alternations
	: alternations '|' cp
	  {
	    $<voidptr>$ = makeNode (RE_OR, $<voidptr>1, $<voidptr>3);
	  }
	| cp
	  {
	    $<voidptr>$ = $<voidptr>1;
	  }
	;

sequences
	: sequences ',' cp
	  {
	    $<voidptr>$ = makeNode (RE_CAT, $<voidptr>1, $<voidptr>3);
	  }
	| cp
	  {
	    $<voidptr>$ = $<voidptr>1;
	  }
	;

cp
	: exp '?'
	  {
	    /* exp? = exp|empty */
	    $<voidptr>$ = makeNode (RE_OR, $<voidptr>1, emptyNode);
	  }
	| exp '*'
	  {
	    $<voidptr>$ = makeNode (RE_STAR, $<voidptr>1, NULL);
	  }
	| exp '+'
	  {
	    /* r+ = rr* */
	    $<voidptr>$ = makeNode (RE_CAT, $<voidptr>1, makeCopyNode (RE_STAR, $<voidptr>1, NULL));
	  }
	| exp
	  {
	    $<voidptr>$ = $1;
	  }
	;

exp
	: NAME
	  { 
	    $<voidptr>$ = makeLeaf ($<voidptr>1);
	  }
	| '(' alternations ')'
	  {
	    $<voidptr>$ = $<voidptr>2; 
	  }
	| '(' sequences ')'	
	  {
	    $<voidptr>$ = $<voidptr>2; 
	  }
	| error			{ _error("illegal content spec."); OHBOY; }
	;

pcd_follows
	: pcd_follows pcd_follow
	| pcd_follow
	| error			{ _error("illegal #PCDATA spec."); OHBOY; }
	;

pcd_follow
	: '|' NAME
	;


/*
 * <!ATTLIST elem-name attributes>
 */
AttlistDecl
	: LT_EX_ATTLIST NAME 
	  {
	    extern int attribTab;  /* see saxparser.c */

	    if ($2 == NULL)
	      warn ("AttlistDecl: NAME was NULL");
	    else {
	      if ((current_element = (Element*) lookupData (attribTab, $2)) == NULL) {
		current_element = newElementStruct ($2);
		insertData (attribTab, $2, current_element);
	      }
	      saveSymbolAndData (attribTab, $2, current_element);
	    }
	  }
	  AttDefs '>'
          {
	    current_element = NULL;
	  }
	| LT_EX_ATTLIST NAME error	{ _error("illegal ATTLIST spec. Wrong attribute def."); OHBOY; }
	| LT_EX_ATTLIST error		{ _error("illegal ATTLIST spec. Missing NAME."); OHBOY; }
	;

AttDefs
	: AttDefs AttDef
	| AttDef
	;

AttDef
	: NAME 
	  {
	    current_attribute = newAttributeStruct ($1);
	    addAttributeToElement (current_element, current_attribute);
	  }
	  AttType DefaultDecl
	  {
	    setAttributeType (current_attribute, $3, $4);
	  }
	;

AttType
	: CDATA				{ $$ = AT_CDATA; }
	| ID				{ $$ = AT_ID; }
	| IDREF				{ $$ = AT_IDREF; }
	| IDREFS			{ $$ = AT_IDREFS; }
	| ENTITY			{ $$ = AT_ENTITY; }
	| ENTITIES			{ $$ = AT_ENTITIES; }
	| NMTOKEN			{ $$ = AT_NMTOKEN; }
	| NMTOKENS			{ $$ = AT_NMTOKENS; }
	| '(' attenum ')'		{ $$ = AT_ENUM; }
	| NOTATION '(' attenum ')'	{ $$ = AT_NOTATION; }
	;

DefaultDecl
	: S_REQUIRED			{ $$ = AX_REQUIRED; }
	| S_IMPLIED			{ $$ = AX_IMPLIED; }
	| S_FIXED QuotedString
	  { 
	    setDefaultToAttribute (current_attribute, saveName ($2));
	    $$ = AX_FIXED; 
	  }
	| QuotedString
	  { 
	    setDefaultToAttribute (current_attribute, saveName ($1));
	    $$ = AX_DEFAULT;	/* use default if attribute not present */
	  }
	;

attenum
	: attenum '|' NAME	{ addEnumNameToAttribute (current_attribute, $3); }
	| NAME			{ addEnumNameToAttribute (current_attribute, $1); }
	| error			{ _error("illegal ATTLIST enumeration spec.."); OHBOY; }
	;

/*
 * <!ENTITY
 */
EntityDecl
	: LT_EX_ENTITY NAME EntityDef '>'  
	  {
	    /*GEDecl*/
	    saveEntity ($2, $3);
	  }
	| LT_EX_ENTITY NAME EntityDef error	{ _error("illegal ENTITY spec. Missing '>'."); OHBOY; }
	| LT_EX_ENTITY NAME error		{ _error("illegal ENTITY spec."); OHBOY; }
	| LT_EX_ENTITY error			{ _error("illegal ENTITY spec. Missing entity name."); OHBOY; }
	| LT_EX_ENTITY '%' NAME PEDef '>' 
	  {
	    /*PEDecl ほんとうはEntityDef->PEDef*/
	    saveEntity ($3, $4);
	  }
	| LT_EX_ENTITY '%' NAME PEDef error	{ _error("illegal ENTITY spec. Missing '>'."); OHBOY; }
	| LT_EX_ENTITY '%' NAME error		{ _error("illegal ENTITY spec."); OHBOY; }
	| LT_EX_ENTITY '%' error		{ _error("illegal ENTITY spec. Missing entity name."); OHBOY; }
	;

EntityDef
	: EntityValue		{ $$ = $1; }
	| ExternalID		{ $$ = $1; }
	| ExternalID NDataDecl	{ $$ = $1; }
	;

PEDef
	: EntityValue		{ $$ = $1; }
	| ExternalID		{ $$ = $1; }
	;

/*
 * この文字列の中にはリファレンス(Reference|PEReference)がふくまれてよい。
 *  したがってプログラム的に、リファレンスを変換する必要がある。
 */
EntityValue
	: QuotedString		{ $$ = $1; }
	;
	
NDataDecl
	: NDATA QuotedString
	;

/*
 * <!NOTATION
 */
NotationDecl
	: LT_EX_NOTATION NAME SYSTEM SystemLiteral '>'
	| LT_EX_NOTATION NAME SYSTEM SystemLiteral error { _error("illegal NOTATION spec. Missing '>'."); OHBOY; }
	| LT_EX_NOTATION NAME SYSTEM error	{ _error("illegal NOTATION spec."); OHBOY; }
	| LT_EX_NOTATION NAME PUBLIC PublicLiteral '>'
	| LT_EX_NOTATION NAME PUBLIC PublicLiteral error { _error("illegal NOTATION spec. Missing '>'."); OHBOY; }
	| LT_EX_NOTATION NAME PUBLIC error	{ _error("illegal NOTATION spec. Missing '>'."); OHBOY; }
	| LT_EX_NOTATION NAME error		{ _error("illegal NOTATION spec. Missing URI."); OHBOY; }
	| LT_EX_NOTATION error		{ _error("illegal NOTATION spec. Missing external id (uri)."); OHBOY; }
	;

/*
 *  !!!following lines are moved to dtd.y!!!
 * External Subset = external DTD file
 *
 *extSubset
 *	: TextDecl extSubsetDecl
 *	| extSubsetDecl
 *	;
 *
 *TextDecl
 *	: LT_Q_XML VersionInfo EncodingDecl '?' '>'
 *	| LT_Q_XML EncodingDecl '?' '>'
 *	| LT_Q_XML EncodingDecl error				{ _error(""); OHBOY; }
 *	;
 *
 *extSubsetDecl
 *	: extSubsetDecl extsub
 *	| extsub
 *	;
 *
 *extsub
 *	: markupdecl
 *	| conditionalSect
 *	| PEREF
 *	;
 *
 *
 * conditional <![IGNORE[...]]>  <![INCLUDE[...]]>
 * 
 *conditionalSect
 *	: LT_EX_SB IGNORE '[' { begin_cdata(); } CDATA_REST ** ignoreSect **
 *	| LT_EX_SB IGNORE '[' { begin_cdata(); } error		{ _error(""); OHBOY; }
 *	| LT_EX_SB IGNORE error					{ _error(""); OHBOY; }
 *	| LT_EX_SB error					{ _error(""); OHBOY; }
 *	| LT_EX_SB INCLUDE '[' extSubsetDecl  SB_SB_GT   **includeSect**
 *	| LT_EX_SB INCLUDE '[' extSubsetDecl  error		{ _error(""); OHBOY; }
 *	| LT_EX_SB INCLUDE '[' error				{ _error(""); OHBOY; }
 *	| LT_EX_SB INCLUDE error				{ _error(""); OHBOY; }
 *	;
 */
/***********************************************************
 * 以下はDTDとのコンパチビリティのための文法。XMLでは許されていない。
 * 2つのパーサをもつのが面倒なのと、保守がたいへんなのでこうした。
 ***********************************************************/
somemarkupdecls
	: somemarkupdecls somemarkupdecl
	| somemarkupdecl
	;
somemarkupdecl
	: elementdecl
	| AttlistDecl
	| EntityDecl 
	| NotationDecl
	| conditionalSect
	| PI
	  {
	    (*sax_processingInstruction)(getPITarget ($1), getPIData ($1));
	  }
	;

/*
 * conditional <![IGNORE[...]]>  <![INCLUDE[...]]>
 */
/* ###indep### */
conditionalSect
	: LT_EX_SB _IGNORE '[' { begin_cdata(); } CDATA_REST /* ignoreSect */
	| LT_EX_SB _IGNORE '[' { begin_cdata(); } error	{ _error("illegal IGNORE section."); OHBOY; }
	| LT_EX_SB _IGNORE error			{ _error("illegal IGNORE section."); OHBOY; }
	| LT_EX_SB error				{ _error("illegal IGNORE section."); OHBOY; }
	| LT_EX_SB INCLUDE '[' extSubsetDecl  SB_SB_GT   /*includeSect*/
	| LT_EX_SB INCLUDE '[' extSubsetDecl  error	{ _error("illegal INCLUDE section."); OHBOY; }
	| LT_EX_SB INCLUDE '[' error			{ _error("illegal INCLUDE section."); OHBOY; }
	| LT_EX_SB INCLUDE error			{ _error("illegal INCLUDE section."); OHBOY; }
	;
/* ###indep### */
extSubsetDecl
	: extSubsetDecl extsub
	| extsub
	;
/* ###indep### */
extsub
	: markupdecl
	/*| conditionalSect*/
	| PEREF
	;


%%

extern int LINE_NO;
static char msgbuf[256];

/*
 */
static void yyerror (char* msg)
{
  sprintf(msgbuf, "@LINE[%d]: %s", LINE_NO, msg);
  (*sax_fatalError)(msgbuf);
}

/*
 */
static void _error (char* msg)
{
  sprintf(msgbuf, "@LINE[%d]: %s", LINE_NO, msg);
  (*sax_error)(msgbuf);
}

/*
 */
void warn (char* msg)
{
  sprintf (msgbuf, "@LINE[%d]: %s", LINE_NO, msg);
  (*sax_warning)(msgbuf);
}

/*
 */
void warn2 (char* msg1, char* msg2)
{
  sprintf (msgbuf, "@LINE[%d]: %s%s", LINE_NO, msg1, msg2);
  (*sax_warning)(msgbuf);
}

/*
 */
void warn3 (char* msg1, char* msg2, char* msg3)
{
  sprintf (msgbuf, "@LINE[%d]: %s%s%s", LINE_NO, msg1, msg2, msg3);
  (*sax_warning)(msgbuf);
}


#if 0
void main (int ac, char* av[]) {
  extern int yydebug;
  FILE* file;
  yydebug = 1;

  if (file = fopen (av[1], "r")) {
    parseXml (file);
    fclose (file);
  }
}
#endif
/* end */


