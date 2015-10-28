/*
 *  intset.h
 *
 *  正の整数の集合
 *
 *
 */
#ifndef INTSET_H
#define INTSET_H

#ifndef DEFAULTS_H
# include "defaults.h"
#endif

typedef unsigned char bits8;

#ifndef TRUE
#define TRUE   1
#define FALSE  0
#endif

#ifndef NULL
#define NULL   0
#endif


/*
 * 256bits that means this set can only
 * have integers 0 to 255. 255 states only.
 */
#define BITSBYTES (MAX_BITS/8)


typedef struct {
  short   capacity;
  short   maximum;  /* number of items */
  short   curr;    /* working variable for iteration */
  unsigned char bits[BITSBYTES];
} _Intset, *Intsetp ;


Intsetp   NewIntset (unsigned int capacity);
Intsetp   NewIntsetAt (unsigned int capacity, Intsetp is);
void      ISreset (Intsetp); 
void      ISrelease (Intsetp); 
void      ISaddItem (Intsetp, unsigned);
void      ISaddAllItems (Intsetp, Intsetp);
void      ISremoveItems (Intsetp, unsigned);
int       ISisInSet (Intsetp, unsigned);
void      ISiterate (Intsetp);        
int       ISnext (Intsetp);        
int       ISnextNot (Intsetp);     
void      ISiterateRev (Intsetp);        
int       ISnextNotRev (Intsetp);     
int       ISisEqual (Intsetp, Intsetp);     
Intsetp   ISclone (Intsetp src, Intsetp dest);
int       ISisEmpty (Intsetp set);

#endif


