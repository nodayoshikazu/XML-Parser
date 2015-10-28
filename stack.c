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
 * ���������å�����դ�
 */

XStack* newXStack (void)
{
  XStack* st = (XStack*) getmem (sizeof (XStack));
  st->sp = 0;
  return st;
}

/*
 * �����å�����
 */

void releaseXStack (XStack* xs)
{
  memfree (xs);
}

/*
 * �ץå���
 */

void xpush (XStack* xs, void* s)
{
  if (xs->sp == STACKSIZE)
    warn ("XStack overflow");	/* see xml.y */
  else
    xs->stack[xs->sp++] = s;
}

/* 
 * �ݥå�
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
 * �ݥå�
 */

void* xtop (XStack* xs)
{
  if (xs->sp == 0) {
    /*_warning ("XStack is empty");*/
    return NULL;
  }
  return xs->stack[xs->sp-1];
}
