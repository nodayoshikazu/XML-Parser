/*
 *
 *    setset.h
 *
 *   set of intset
 *
 *  07/16/01
 *
 */
#include "mem.h"
#include "intset.h"
#include "setset.h"
#include "debug.h"


/* 
 * ����
 */

Setsetp NewSetSet ()
{
  Setsetp p = (Setsetp) getmem (sizeof (_Setset));
  p->nsets = 0;
  return p;
}

/*
 * ����
 */

void SSrelease (Setsetp ss)
{
  int i;

  for (i = 0; i < ss->nsets; i++)
    memfree ((char*)ss->sets[i]);
  memfree ((char*)ss);
}

/*
 * ���åȤ򸡺����롣����ǥå������֤���
 *  ���Ĥ���ʤ�����-1���֤���
 */

int SSsearch (Setsetp ss, Intsetp is)
{
  int i;

  for (i = 0; i < ss->nsets; i++) {
    if (ISisEqual (ss->sets[i], is)) 
      return i;
  }
  return -1;	/* not found */
}

/*
 * ���åȤ��ɲä��롣�ɲä�������ǥå������֤���
 */

int SSadd (Setsetp ss, Intsetp is)
{
  if (is->maximum == 0) {
    /* never add null set */
    return -1;
  }
  ss->sets[ss->nsets] = is;
  ss->nsets++;
  return ss->nsets-1;
}

/*
 * ����ǥå����Υ��åȤ��֤���
 */

Intsetp SSget (Setsetp ss, int index) { 
  if (ss->nsets <= index)
    _fatal ("SSget() index out of range.");
  return ss->sets[index]; 
}

/*
 *
 */

int      SSgetNumSets (Setsetp ss) { return ss->nsets; }

#if 0
/*
 *
 */

Intsetp* SSgetSets (Setsetp ss) { return ss->sets; }

int SSgetSymbol (Setsetp ss, int index) {
  if (ss->nsets <= index)
    _fatal ("SSgetSymbol() index out of range.");
  return ss->onsymbols[index];
}

/*
 * returns new set's index if it doesn't exist and added.
 *  -1 otherwise.
 */

int SSaddSetIfNotExist (Setsetp ss, Intsetp is, int onsymbol)
{
  int i, r;

  if (is->maximum == 0) {
    /* never add null set */
    return -1;
  }
  if (ss->nsets == 0) {
    /* if no sets now */
    ss->sets[ss->nsets] = is;
    ss->onsymbols[ss->nsets] = onsymbol;
    ss->nsets++;
    r = ss->nsets-1;
  }
  else {
    int eq = 0;
    for (i = 0; i < ss->nsets; i++) {
      if (ISisEqual (ss->sets[i], is)) {
	eq++;
	ss->onsymbols[i] = onsymbol;
      }
    }
    if (eq == 0) {
				/* no same set */
      ss->sets[ss->nsets] = is;
      ss->onsymbols[ss->nsets] = onsymbol;
      ss->nsets++;
      r = ss->nsets-1;
    }
  }
  return r;			/* already there */
}
#endif
