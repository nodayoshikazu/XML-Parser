/*
 *
 *    stack.h
 *
 *
 *
 *
 *
 *   08/10/01
 */
#ifndef STACK_H
#define STACK_H

#ifndef DEFAULTS_H
# include "defaults.h"
#endif


typedef struct {
  int    sp;
  void*  stack[STACKSIZE];
} XStack;

XStack* newXStack (void);
void    releaseXStack (XStack* xs);
void    xpush (XStack* xs, void* s);
void*   xpop (XStack* xs);
void*   xtop (XStack* xs);

#endif
