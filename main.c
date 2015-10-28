/*
 *
 *    main.c
 *
 *
 *
 *
 */
#include <stdio.h>
#include "debug.h"
#include "saxhandler.h"



void startDocument (void)
{
  printf ("============startDocument============\n");
}

void endDocument (void) 
{
  printf ("============endDocument============\n");
}

void printValue (StrTreeNode node) {
  char* value = (char*)(Closure(node));
  if (value)
    printf ("%s = %s\n", Key(node), value);
  else
    printf ("%??? = NULL...this is not a problem\n");
}

void startElement (char* uri, char* localName, char* qName, Attributes attr) {
  if (uri == NULL)
    uri = "NULL";
  printf ("startElement uri=[%s] localName=[%s] qName=[%s]\n", uri, localName, qName);
  STreeApplyNode (attr, printValue);
}

void characters (char* chars, int start, int length) {
  if (chars)
    printf ("characters [%s]\n", chars);
  else
    printf ("characters [*NULL*]\n", chars);
}

void endElement (char* uri, char* localName, char* qName) {
  if (uri == NULL)
    uri = "NULL";
  printf ("endElement uri=[%s] localName=[%s] qName=[%s]\n", uri, localName, qName);
}

void startPrefixMapping (char* prefix, char* uri){
  if (prefix == NULL)
    prefix = "@@@@ BAD NULL";
  if (uri == NULL)
    uri = "NULL";
  printf ("startPrefixMapping...prefix=%s uri=%s\n", prefix, uri);
}

void endPrefixMapping (char* prefix) {
  if (prefix == NULL)
    prefix = "@@@@ BAD NULL";
  printf ("endPrefixMapping...prefix=%s\n", prefix);
}

void procInst (char* t, char* d)
{
  printf ("processingInstruction [%s] [%s]\n", t, d);
}

void fatalerror (char* msg)
{
  printf ("Fatal error: %s\n", msg);
}

void error (char* msg)
{
  printf ("Error: %s\n", msg);
}

void warning (char* msg)
{
  printf ("Warning: %s\n", msg);
}

void main (int ac, char* av[]) {
  sax_startDocument = startDocument;
  sax_endDocument = endDocument;
  sax_startElement = startElement;
  sax_endElement = endElement;
  sax_characters = characters;
  sax_startPrefixMapping = startPrefixMapping;
  sax_endPrefixMapping = endPrefixMapping;
  sax_processingInstruction = procInst;
  sax_fatalError = fatalerror;
  sax_error = error;
  sax_warning = warning;

  parse_xml (av[1]);
}

