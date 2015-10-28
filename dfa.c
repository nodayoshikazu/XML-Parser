/*
 *
 *   dfa.c
 *
 *
 *
 *   08/09/01
 *
 */
#include <stdlib.h>
#include "intset.h"
#include "setset.h"
#include "dfa.h"
#include "mem.h"
#include "debug.h"

#define CHKMEM 1
/*#define DBGDFA 1*/



/* last (accepting) node for augumented grammer */
static RESyntaxTreeNode _lastNode = {
  RE_LEAF,			/* op */
  ACCEPT_STATE,			/* num=fixed */
  "#",				/* symbol */
  NULL,				/* left */
  NULL,				/* right */
  FALSE				/* nullable */
};

/* Needed for x? grammer */
static RESyntaxTreeNode _emptyNode = {
  RE_EMPTY,			/* op */
  EMPTY_STATE,			/* nul..fixed */
  "@",				/* symbol */
  NULL,				/* left */
  NULL,				/* right */
  TRUE				/* nullable */
};

REN emptyNode = &_emptyNode;	/* アクセプトシンボル */
REN lastNode = &_lastNode;	/* 空シンボル */

#ifdef CHKMEM
static int memcount = 0;
static int rencount = 0;
#endif

static int numberOfLeaves;

static REN  leaves[MAX_LEAVES];
static int  n_leaves;
static Goto gotobuf[MAX_GOTOS];
static int  nGotos;


/*
 * 新ノードメモリー割り付け
 */

static REN newNode ()
{
#ifdef CHKMEM
  memcount += sizeof (RESyntaxTreeNode);
#endif
  return (REN) getmem (sizeof (RESyntaxTreeNode));
}

/*
 * 初期化(DFAを作る際に毎回呼び出す必要ある)
 */

void initDfa ()
{
  n_leaves = 0;
  leaves[n_leaves++] = lastNode;
  leaves[n_leaves++] = emptyNode;
  numberOfLeaves = STARTSTATE;
}

/*
 * シンタックスツリーを解放
 */

REN releaseSyntaxTree (REN node)
{
  if (node) {
    node->left = releaseSyntaxTree (node->left);
    node->right = releaseSyntaxTree (node->right);
    if (node != &_lastNode && node != &_emptyNode) {
      memfree ((char*)node);
#ifdef CHKMEM
      --rencount;
#endif
    }
  }
  return NULL;
}

/*
 * シンタックスツリーのリーフノードを作る
 */

REN makeLeaf (char* symbol) 
{
  REN ren = newNode ();

#ifdef CHKMEM
  ++rencount;
#endif
  ren->op = RE_LEAF;
  ren->num = numberOfLeaves++;
  ren->symbol = symbol;
  ren->left = NULL;
  ren->right = NULL;
  ren->nullable = FALSE;
  leaves[n_leaves++] = ren;
  return ren;
}

/*
 * シンタックスツリーのノードを作る
 */

REN makeNode (int op, REN left, REN right)
{
  REN ren = newNode ();

#ifdef CHKMEM
  ++rencount;
#endif
  ren->op = op;
  ren->symbol = NULL;
  ren->left = left;
  ren->right = right;
  return ren;
}

/*
 * シンタックスツリーのノードを作る
 */

static REN copyNode (REN node)
{
  REN r = NULL;

  if (node) {
    if (node->op == RE_LEAF)
      r = makeLeaf (node->symbol);
    else
      r = makeCopyNode (node->op, node->left, node->right);
  }
  return r;
}

/*
 * Leafでないノードのコピーを作る
 */

REN makeCopyNode (int op, REN left, REN right)
{
  REN ren = newNode ();

#ifdef CHKMEM
  ++rencount;
#endif
  ren->op = op;
  ren->symbol = NULL;
  ren->left = copyNode (left);
  ren->right = copyNode (right);
  return ren;
}

/*
 * アクセプトステートを追加して正規表現をaugumentする。
 */

static REN addLastSymbol (REN node)
{
  return makeNode (RE_CAT, node, lastNode);
}

/*
 * Dragon Book pp 138
 */

static int nullable (REN node)
{
  switch (node->op) {
  case RE_EMPTY: 
    node->nullable = TRUE;
    break;
  case RE_LEAF: 
    node->nullable = FALSE;
    break;
  case RE_CAT: 
    node->nullable = node->left->nullable & node->right->nullable;
    break;
  case RE_STAR:
    node->nullable = TRUE;
    break;
  case RE_OR:
    node->nullable = node->left->nullable | node->right->nullable;
    break;
  }
  return node->nullable;
}

/*
 * Dragon Book  pp 138
 */

static void firstpos (REN node)
{
  switch (node->op) {
  case RE_EMPTY: 
    /* nil */
    break;
  case RE_LEAF: 
    ISaddItem (&node->firstpos, node->num);
    break;
  case RE_CAT: 
    if (node->left->nullable == TRUE) {
      ISaddAllItems (&node->firstpos, &node->left->firstpos);
      ISaddAllItems (&node->firstpos, &node->right->firstpos);
    }
    else
      ISaddAllItems (&node->firstpos, &node->left->firstpos);
    break;
  case RE_STAR:
    ISaddAllItems (&node->firstpos, &node->left->firstpos);
    break;
  case RE_OR: 
    ISaddAllItems (&node->firstpos, &node->left->firstpos);
    ISaddAllItems (&node->firstpos, &node->right->firstpos);
    break;
  }
}

/*
 * Dragon Book  pp 138
 */

static void lastpos (REN node)
{
  switch (node->op) {
  case RE_EMPTY: 
    /* nil */
    break;
  case RE_LEAF: 
    ISaddItem (&node->lastpos, node->num);
    break;
  case RE_CAT: 
    if (node->right->nullable == TRUE) {
      ISaddAllItems (&node->lastpos, &node->left->lastpos);
      ISaddAllItems (&node->lastpos, &node->right->lastpos);
    }
    else
      ISaddAllItems (&node->lastpos, &node->right->lastpos);
    break;
  case RE_STAR:
    ISaddAllItems (&node->lastpos, &node->left->lastpos);
    break;
  case RE_OR: 
    ISaddAllItems (&node->lastpos, &node->left->lastpos);
    ISaddAllItems (&node->lastpos, &node->right->lastpos);
    break;
  }
}

/* 
 * ツリーを巡回してfirstpos(node)と lastpos(node)を設定する
 */

static void setFirstposAndLastpos (REN node)
{
  if (node != NULL) {
    setFirstposAndLastpos (node->left);
    setFirstposAndLastpos (node->right);

    /* firstpos */
    NewIntsetAt (numberOfLeaves, &node->firstpos);
    firstpos (node);

    /* lastpos */
    NewIntsetAt (numberOfLeaves, &node->lastpos);
    lastpos (node);

    /* followpos */
    NewIntsetAt (numberOfLeaves, &node->followpos); /* for later use */
  }
}

/*
 * ツリーを巡回してfollowpos(leaf-node)を設定
 */

static void setNullable (REN node)
{
  if (node) {
    setNullable (node->left);
    setNullable (node->right);
    nullable (node);
  }
}

/*
 * ステートIDからシンボルを得る
 */

static REN searchNodeForState (REN node, int state)
{
  REN r;

  if (!node)
    return NULL;
  if (r = searchNodeForState (node->left, state))
    return r;
  if (node->num == state)
    return node;
  return searchNodeForState (node->right, state);    
}

/*
 * find the followpos sets
 * Dragon book pp 138
 */

static void findFollowpos (REN node, REN root) 
{
  int p;
  REN n;

  if (node != NULL) {
    if (node->op == RE_CAT) {
      Intsetp set = &node->left->lastpos; /* lastpos(c1) */
      ISiterate (set);
      while ((p = ISnext (set)) > -1) {
	/* for all positions in lastpos(c1) 
	 */
	n = searchNodeForState (root, p);
	ISaddAllItems (&n->followpos, &node->right->firstpos); /* firstpos(c2) */
      }
    }
    if (node->op == RE_STAR) {
      Intsetp set = &node->lastpos; /* lastpos(c1) */      
      ISiterate (set);
      while ((p = ISnext (set)) > -1) {
	/* for all positions in lastpos(n) 
	 */
	n = searchNodeForState (root, p);	
	ISaddAllItems (&n->followpos, &node->firstpos); /* firstpos(c2) */
      }
    }
  }      
}

/*
 * ツリーを巡回してfollowpos(leaf-node)を設定
 */

static void setFollowpos (REN node, REN root)
{
  if (node) {
    setFollowpos (node->left, root);
    setFollowpos (node->right, root);
    findFollowpos (node, root);
  }
}


/*
 * Dfaデータを作成する
 */

static Dfa* newDfaStruct (int nSymbols, int nStates, int nGoto, Goto* gotos)
{
  Dfa* dfa;
  int  i;

#ifdef CHKMEM
  memcount += (sizeof (Dfa) + (nSymbols * sizeof(char*)) + (nGoto * sizeof(Goto)));
#endif
  dfa = (Dfa*) getmem (sizeof (Dfa) + (nSymbols * sizeof(char*)) + (nGoto * sizeof(Goto)));
  dfa->n_symbols = nSymbols;
  dfa->n_states = nStates;
  dfa->n_gotos = nGoto;
  dfa->gotos = (Goto*)dfa->bottom;
  for (i = 0; i < nGoto; i++)
    dfa->gotos[i] = gotos[i];
  return dfa;
}

/*
 * 重複をさけて追加
 */

static addGoto (int fromState, char* symbol, int toState, int accept)
{
  int i, found;

  /*
   * 実際には空文字(epsilon)とか、#は入力されないので、gotoテーブルには入れない。
   */
  if (*symbol != '#' && *symbol != '@') {
    found = FALSE;
    for (i = 0; i < nGotos; i++)
      if (gotobuf[i].fromState == fromState)
	break;
    for (; gotobuf[i].fromState == fromState && i < nGotos; i++) {
      if (gotobuf[i].input_symbol == symbol && gotobuf[i].toState == toState) {
	found = TRUE;
	break;
      }
    }
    if (!found) {
      gotobuf[nGotos].fromState = fromState;
      gotobuf[nGotos].input_symbol = symbol;
      gotobuf[nGotos].toState = toState;
      gotobuf[nGotos].accept = accept;
      nGotos++;
    }
  }
}

/*
 *  create_dfa (root);
 *    ref: Dragon Book pp 141.
 *
 *  DFAの姿:
 *  シンボル配列 [ptr][ptr2]...[ptrn] 
 *  これはSyntax TreeのLeafだけを集めれば出来る。
 *  配列上のシンボルのインデックスがstate番号になる。
 *
 *  DFAは２次元配列。縦がカレントステート。横がシンボルインデックス。
 *  中身がgotoステート。
 *
 *     | 0 | 1 | 2 | 3 | 4 | 5 | <-- symbol index
 *  ---+---------------------------
 *  0  | x | y | z | z | x | y |     <--- goto state
 *  ---+---------------------------
 *  1  | x | y | z | z | x | y |
 *  ---+---------------------------
 *  2  | x | y | z | z | x | y |
 *  ---+---------------------------
 *  3  | x | y | z | z | x | y |
 *  ---+---------------------------
 *  4  | x | y | z | z | x | y |
 *  ---+---------------------------
 *  5  |
 *
 *  これを1次元配列で作成。配列のポインターと、インデックス数、ステート数を持てばいい。
 *
 *  持ちもの
 *  1) シンボル配列
 *  2) シンボル配列のシンボル数=インデックス数
 *  3) ステート数。(アクセプトステートのインデックスだけわかるように)
 *     ＝Dtransのセット数
 *  4) goto配列
 */

static Dfa* create_dfa (REN root)
{
  int     i, j, k, p, accept;
  Setsetp Dtrans;
  Intsetp tmpIS;
  Dfa*    dfa = NULL;

  /* 初期ステート */
  nGotos = 0;
  Dtrans = NewSetSet ();
  SSadd (Dtrans, ISclone (&root->firstpos, NewIntset (numberOfLeaves)));

  /* create a temporary intset for merging 
   */
  tmpIS = NewIntset (numberOfLeaves);
  /*
   * Dtransはループの中で増える
   */
  for (i = 0; i < SSgetNumSets (Dtrans); i++) {
    Intsetp isetp = SSget (Dtrans, i);

    for (j = 0; j < n_leaves; j++) {
      ISreset (tmpIS);
      ISiterate (isetp);
      /* 
       * Do for all symbols in Dtrans[i]
       */
      while ((p = ISnext (isetp)) > -1) {
	/* strcmp()の必要なし */
	if (leaves[j]->symbol == leaves[p]->symbol) {
	  /*
	   * If symbol-j = symbol-p, add all positions in followpos(p)
	   */
	  REN node = leaves[p];
	  ISaddAllItems (tmpIS, &node->followpos);
#ifdef DBGDFA
	  ISprint (&node->followpos); printf("...followpos\n");
#endif
	}
      }
#ifdef DBGDFA
      ISprint (tmpIS); printf("==tmpIS...\n");
      for (k = 0; k < SSgetNumSets (Dtrans); k++) {
	Intsetp isetp = SSget (Dtrans, k);
	ISprint (isetp); 
	printf("---a set in Dtrans\n");
      }
#endif
      /*
       * Dtransの中にtmpISと同じセットがあるかどうか探す
       *  なければ、Dtransに追加する。
       *  iが開始ステートで、kは行き先ステート
       */
      if ((k = SSsearch (Dtrans, tmpIS)) < 0) {
	/* same set is not found 
	 */
	Intsetp nis;

	if (!ISisEmpty (tmpIS)) {
	  if (SSadd (Dtrans, ISclone (tmpIS, nis = NewIntset (numberOfLeaves))) < 0)
	    ISrelease (nis);	/* just in case??? */
	}
      }
      else {
	accept = ISisInSet(tmpIS, ACCEPT_STATE);
	addGoto (i, leaves[j]->symbol, k, accept);
      }
    }
  }
  /*
   * DFAを作成
   */
  dfa = newDfaStruct (n_leaves, SSgetNumSets (Dtrans), nGotos, gotobuf);
  ISrelease (tmpIS);		/* free tmpIS */
  SSrelease (Dtrans);		/* free Dtrans */
  return dfa;
}

#ifdef DEBUG
/*
 */

static char* toOpString (int op) {
  switch (op) {
  case RE_EMPTY: return "empty";
  case RE_LEAF: return "leaf";
  case RE_CAT: return ".";
  case RE_STAR: return "*";
  case RE_OR: return "|";
  }
}

/*
 */

static rprint (REN node)
{
  if (node != NULL) {
    char *sym;

    rprint (node->left);
    rprint (node->right);
    sym = node->symbol == NULL ? "NULL" : node->symbol;
    printf ("[node=%08x][num=%d][op=%s][symbol=%s][left=%08x][right=%08x][nullable=%d]",
	    node, node->num, toOpString(node->op), sym, node->left, node->right, node->nullable);
    ISprint (&node->firstpos);
    ISprint (&node->lastpos);
    ISprint (&node->followpos);
    printf ("\n");
  }
}

/*
 */

static void testview (REN node)
{
  printf ("ROOT=<%08x>\n", node);
  rprint (node);
}
#endif

/*
 * DFA creation
 */

Dfa* createDFA (REN root)
{
  Dfa* dfa;

  root = addLastSymbol (root);
  setNullable (root);
  setFirstposAndLastpos (root);
  setFollowpos (root, root);

  DBGIN(testview (root);)	/* debug */
  /*DBGIN(for (i = 0; i < n_leaves; i++) printf ("[%s]\n", leaves[i]->symbol);)*/

  dfa = create_dfa (root);
  releaseSyntaxTree (root);
#if 0
  printf ("........memcount=%d rencount=%d\n", memcount, rencount);
#endif
  return dfa;
}

/*
 *
 */

void releaseDFA (Dfa* dfa)
{
  if (dfa)
    memfree ((char*) dfa);
}

#ifdef DEBUG
/*
 * print for debug
 */

void printDfa (Dfa* dfa) 
{
  int i;

  for (i = 0; i < dfa->n_gotos; i++) {
    char* xx = "NULL";

    if (dfa->gotos[i].input_symbol)
      xx = dfa->gotos[i].input_symbol;
    printf ("From state [%d], On <%s> goto [%d]...accept{%d}\n", 
	    dfa->gotos[i].fromState,
	    xx,   /*dfa->gotos[i].input_symbol, */
	    dfa->gotos[i].toState,
	    dfa->gotos[i].accept);
  }
}

#endif
