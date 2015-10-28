/*
 *  intset.c
 *
 *
 *
 *
 *   2001/07/13
 */
#include <stdio.h>
#include <malloc.h>
#include "intset.h"
#include "mem.h"


static bits8 masks[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };



/*
 * max is 128
 */
Intsetp NewIntset (unsigned int capacity)
{
  register Intsetp s;

  if ((s = (Intsetp) getmem (sizeof (_Intset))) == NULL)
    _fatal("getmem@ISnew");
  memset (s, 0, sizeof(_Intset));
  s->capacity = capacity;
  s->maximum = 0;
  s->curr = 0;
  return s;
}

/*
 * Create one at supplied address. max is 128
 */

Intsetp NewIntsetAt (unsigned int capacity, Intsetp is)
{
  memset (is, 0, sizeof(_Intset));
  is->capacity = capacity;
  is->maximum = 0;
  is->curr = 0;
  return is;
}

/*
 */
int  ISisEmpty (Intsetp set) { return set->maximum == 0; }

void ISrelease (Intsetp set) { memfree (set); }

void ISreleaseAt (Intsetp set) { /* compatibility */ }

void ISreset (Intsetp set) 
{ 
  int cap = set->capacity;
  memset ((char*)set, 0, sizeof(_Intset));
  set->capacity = cap;
}

/*
 */

void ISaddItem (Intsetp set, unsigned integer)
{ 
  set->bits[integer/8] |= masks[integer % 8];
  integer++;
  set->maximum = set->maximum < integer ? integer : set->maximum;
}

/*
 */

void ISaddAllItems (Intsetp set, Intsetp set2)
{
  register int i;

  if (0 < set2->maximum) {
    for ( i = 0; i < BITSBYTES; i++ )
      set->bits[i] |= set2->bits[i];
    set->maximum = set->maximum < set2->maximum ? set2->maximum : set->maximum;
  }
}

/*
 */

void ISremoveItems (Intsetp set, unsigned  item_number)
{ 
  set->bits[item_number/8] &= ~(masks[item_number % 8]); 
}

/*
 */

int ISisInSet (Intsetp set, unsigned item_number)
{
  return (set->bits[item_number/8] & masks[item_number % 8]) != 0; 
}

/*
 */

void ISiterate (Intsetp set)
{
  set->curr = 0; 
}

/*
 */

int ISnext (Intsetp set)
{
  while ( !(set->bits[set->curr/8] & masks[set->curr%8]) && (set->curr < set->capacity))
    set->curr++;
  if ( set->curr == set->capacity )
    return -1;  /* ½ª¤ê */
  return set->curr++;
}

/*
 */

int ISisEqual (Intsetp set, Intsetp set2)
{
  register int i;
  for ( i = 0; i < BITSBYTES; i++ )
    if ( set->bits[i] != set2->bits[i] )
      return 0;
  return 1;			/* equal */
}

/*
 */

Intsetp ISclone (Intsetp src, Intsetp dest)
{
  memcpy (dest, src, sizeof (_Intset));
  return dest;
}

/*
 */

void ISprint (Intsetp set)
{
  int r;

  ISiterate (set);
  printf ("{");
  while (0 <= (r = ISnext (set)))
    printf (" %d", r);
  printf ("}");
}

