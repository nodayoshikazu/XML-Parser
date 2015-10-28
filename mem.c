/*
 *
 *     mem.c
 *
 *
 *  10/28/01
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include "mem.h"



#ifdef MEMCHK

static int mcount = 0;
static int verb = 0;

/*
 *
 */

void* MALLOC (int size)
{
  void* a;
  int   realsize;

  realsize = size + sizeof(int);
  a = malloc (realsize);
  mcount += realsize;
  if (verb)
    printf ("$$$ %d bytes MALLOCed at %08x\n", realsize, a);
  *(int*)a = realsize;
  return (void*)((char*)a + sizeof(int));
}

/*
 */

void FREE (void* address)
{
  char* realaddress;
  int   size;

  realaddress = ((char*)address) - sizeof(int);
  size = *(int*)realaddress;
  mcount -= size;
  if (verb)
    printf ("$$$ %d bytes FREEed at %08x\n", size, realaddress);
  free (realaddress);
}

void imem () { mcount = 0; }
void pmem () { printf ("Memory unfreed now = %d bytes.\n", mcount); }

#else
void pmem (){}
void imem (){}

#endif

void* getmem (int size) 
{
  return (char*) MALLOC (size);
}

char* stringclone (char* s)
{
  if (s) {
    char* p = (char*) getmem (strlen (s) + 1);
    strcpy (p, s);
    return p;
  }
  return NULL;
}

void  memfree (void* mem)
{
  FREE (mem);
}

void* getmemc (int length, int elemsize) 
{
  int size = length * elemsize;
  return (char*) MALLOC (size);
}

void  relmem (void* mem)
{
  FREE (mem);
}





