/*
 * 
 *    stack.c
 * 
 * 
 * 
 *  08/14/01
 * 
 */
#include <stdlib.h>
#include "stack.h"




/*
 * 新規スタック割り付け
 */

XStack* newXStack (void)
{
  XStack* st = (XStack*) getmem (sizeof (XStack));
  st->sp = 0;
  return st;
}

/*
 * スタック解放
 */

void releaseXStack (XStack* xs)
{
  memfree (xs);
}

/*
 * プッシュ
 */

void xpush (XStack* xs, void* s)
{
  if (xs->sp == STACKSIZE)
    warn ("XStack overflow");	/* see xml.y */
  else
    xs->stack[xs->sp++] = s;
}

/* 
 * ポップ
 */

void* xpop (XStack* xs)
{
  if (xs->sp == 0) {
    warn ("XStack underflow");	/* see xml.y */
    return NULL;
  }    
  return xs->stack[--xs->sp];
}

/* 
 * ポップ
 */

void* xtop (XStack* xs)
{
  if (xs->sp == 0) {
    /*_warning ("XStack is empty");*/
    return NULL;
  }
  return xs->stack[xs->sp-1];
}
