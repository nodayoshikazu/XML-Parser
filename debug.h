/*
 *
 *  debug.h
 *
 *
 *  10/18/2001
 */
#ifndef DEBUG_H
#define DEBUG_H


#ifndef  SERIAL_DEBUG
#define DBGWAIT(x)  /* */
#else
#define DBGWAIT(x)   x
#endif

#ifdef DEBUG
#define DBGIN(x)   x
#define DBGOUT(x)  /**/
#define ENTER(name)  printf ("***Entering %s\n", (name))
#define LEAVE(name)  printf ("***Leaving %s\n", (name))
#else

#define DBGIN(x)     /**/
#define DBGOUT(x)    x
#define ENTER(name)  /**/
#define LEAVE(name)  /**/
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

static void __BUG (char* msg) {
  fprintf (stderr, "BUG EXISTS. THIS MUST BE AVOIDED! %s\n", msg);
}

#endif
