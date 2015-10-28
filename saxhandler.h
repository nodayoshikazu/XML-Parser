/* 
 * saxhandler.h
 *
 *
 *
 *
 *  08/23/01
 */
#ifndef SAXHANDLER_H
#define SAXHANDLER_H

#ifndef Strtree_h
# include "strtree.h"
#endif

typedef StrTreeNode Attributes;


#ifdef WIN32
#ifdef USER
#define EXTDLL __declspec(dllimport)
#else  /* not USER */
#define EXTDLL __declspec(dllexport)
#endif
#else  /* unix */
#define EXTDLL extern
#endif


/* content
 */
EXTDLL void (*sax_characters)(char* chars, int start, int length);

/* PI
 */
EXTDLL void (*sax_processingInstruction)(char* target, char* data);

/*
 */
EXTDLL void (*sax_startDocument)(void);

/*
 */
EXTDLL void (*sax_endDocument)(void);

/*
 * uri: DOCTYPE uri
 * localName: element name without namespace prefix
 * qName: namespace prefix + element name
 * attr; attributes
 * nAttributes: number of attributes in "attr"
 */
EXTDLL void (*sax_startElement)(char* uri, char* localName, char* qName, Attributes attr);

/*
 * uri: DOCTYPE uri
 * localName: element name without namespace prefix
 * qName: namespace prefix + element name
 */
EXTDLL void (*sax_endElement)(char* uri, char* localName, char* qName);

/* 
 * namespaceの開始。prefixはnamespace名、uriは、xmlns="uri"のuri
 */
EXTDLL void (*sax_startPrefixMapping)(char* prefix, char* uri);

/* 
 * namespaceの終了。prefixはnamespace名。
 */
EXTDLL void (*sax_endPrefixMapping)(char* prefix);

/*
 */
EXTDLL void (*sax_error)(char* msg);

/*
 */
EXTDLL void (*sax_fatalError)(char* msg);

/*
 */
EXTDLL void (*sax_warning)(char* msg);



EXTDLL void  init_parse_xml (void);
EXTDLL int   parse_xml (char*);
EXTDLL char* getValue (Attributes attr, char* key);

#endif







