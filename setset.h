/*
 *
 *    setset.h
 *
 *   set of intset
 *
 *
 *
 */
#ifndef SETSET_H
#define SETSET_H

#ifndef INTSET_H
# include "intset.h"
#endif
#ifndef DEFAULTS_H
# include "defaults.h"
#endif


typedef struct _Setset {
  short     nsets;
  _Intset*  sets[MAXSETS];
} _Setset, *Setsetp;


Setsetp  NewSetSet ();
void     SSrelease (Setsetp ss);
Intsetp  SSget (Setsetp ss, int index);
int      SSgetNumSets (Setsetp ss);
int      SSsearch (Setsetp ss, Intsetp is);
int      SSadd (Setsetp ss, Intsetp is);

#endif




