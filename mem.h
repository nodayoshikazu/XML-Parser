/*
 *
 *  mem.h
 *
 *
 *   10/18/2001
 */
#ifndef MEM_H
#define MEM_H

#include <memory.h>

#ifdef MEMCHK

void* MALLOC (int size);
void  FREE (void* address);
#else

#define MALLOC malloc
#define FREE   free
#endif

void* getmem (int size);
char* stringclone (char* s);
void  memfree (void* mem);

#endif
