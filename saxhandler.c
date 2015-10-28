/* 
 * saxhandler.c
 *
 *
 *
 *
 *  08/23/01
 */
#define SAXHANDLER_C 1
#include "saxhandler.h"


#ifdef WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT /**/
#endif



/* content
 */
void (*sax_characters)(char* chars, int start, int length);

/* PI
 */
void (*sax_processingInstruction)(char* target, char* data);

/*
 */
void (*sax_startDocument)(void);

/*
 */
void (*sax_endDocument)(void);

/*
 * uri: DOCTYPE uri
 * localName: element name without namespace prefix
 * qName: namespace prefix + element name
 * attr; attributes
 * nAttributes: number of attributes in "attr"
 */
void (*sax_startElement)(char* uri, char* localName, char* qName, Attributes attr);

/*
 * uri: DOCTYPE uri
 * localName: element name without namespace prefix
 * qName: namespace prefix + element name
 */
void (*sax_endElement)(char* uri, char* localName, char* qName);

/* 
 * namespaceの開始。prefixはnamespace名、uriは、xmlns="uri"のuri
 */
void (*sax_startPrefixMapping)(char* prefix, char* uri);

/* 
 * namespaceの終了。prefixはnamespace名。
 */
void (*sax_endPrefixMapping)(char* prefix);

/*
 */
void (*sax_error)(char* msg);

/*
 */
void (*sax_fatalError)(char* msg);

/*
 */
void (*sax_warning)(char* msg);


static void noop() {
  printf ("Ooops!\n");
}

/*
 * initialization
 */

EXPORT void init_parse_xml () 
{
  sax_characters = noop;
  sax_startDocument = noop;
  sax_endDocument = noop;
  sax_endElement = noop;
  sax_error = noop;
  sax_fatalError = noop;
  sax_processingInstruction = noop;
  sax_startElement = noop;
  sax_warning = noop;
  sax_startPrefixMapping = noop;
  sax_endPrefixMapping = noop;
}

/*
 * get attribute value
 */

EXPORT char* getValue (Attributes attribs, char* key)
{
  return STreeSearch (attribs, key);
}
