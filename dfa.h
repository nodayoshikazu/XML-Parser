/*
 *  dfa.h
 *
 *
 *
 *
 *  07/27/01
 */
#ifndef DFA_H
#define DFA_H

#ifndef INTSET_H
# include "intset.h"
#endif
#ifdef DEFAULTS_H
# include "defaults.h"
#endif



#define TRUE  1
#define FALSE 0

#define  RE_EMPTY    0
#define  RE_LEAF     1
#define  RE_CAT      2 
#define  RE_STAR     3
#define  RE_OR       4

#define  ACCEPT_STATE 0
#define  EMPTY_STATE  1
#define  STARTSTATE   2


typedef struct reTreeNode {
  short  op;			/* RE_STAR|RE_CAT|RE_OR */
  short  num;			/* symbol number */
  char*  symbol;
  struct reTreeNode *left;
  struct reTreeNode *right;
  int    nullable;		/* FALSE=NO TRUE=YES */
  _Intset firstpos;		/* firstpos sets ポインターではないのでfreeしない */
  _Intset lastpos;		/* lastpos sets ポインターではないのでfreeしない */
  _Intset followpos;		/* followpos ポインターではないのでfreeしない */
} RESyntaxTreeNode, *REN;



REN makeLeaf (char* symbol);
REN makeNode (int op, REN left, REN right);
REN makeCopyNode (int op, REN left, REN right);


extern REN emptyNode;


typedef struct _Goto {
  short   fromState;
  short   toState;
  char*   input_symbol;
  short   accept;		/* true|false */
} Goto;

typedef struct _Dfa {
  short   n_symbols;
  short   n_states;
  short   n_gotos;
  Goto*   gotos;
  char    bottom[4];
} Dfa;


Dfa* createDFA (REN root);
void releaseDFA (Dfa* dfa);


#endif
