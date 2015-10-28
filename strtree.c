/*
 * strtree.c
 *
 * Red-Black Tree char* as key version
 * Reference: Introduction to ALGORITHMS from MIT Press
 * You must start with a dummy node that contains the minimum
 * key value.
 * This version has a char pointer as the key.
 *
 */
#define _POSIX_C_SOURCE 1
#define _POSIX_SOURCE  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "strtree.h"


#undef XALLOC
#ifdef XALLOC
extern int   __xalloc_init (int p);
extern void* __xalloc (int xa, int size);
extern void  __xdealloc (int xa, void* addr);

static int strtree_xalloc_root;
#define xalloc(size)        (__xalloc(strtree_xalloc_root,size))
#define xdealloc(addr)      (__xdealloc(strtree_xalloc_root,addr))
#endif

/*
 * We use setinel nil representing NULL node
 */

static STNode sentinel = {NULL,NULL,NULL,Black,0,NULL,NULL};
static StrTreeNode nil = &sentinel;

/*
 * Initializer
 */

void
STreeInit (StrTreeNode* root)
{
  *root = nil;
#if 0
  STreeInsert (root, "!", NULL);
  STreeInsert (root, "!!", NULL);
#else
  STreeInsert (root, "\001", NULL);
  STreeInsert (root, "\002", NULL);
#endif
}

/*
 * Get max
 */

void*
STreeSample (StrTreeNode root)
{
  StrTreeNode x = root;

  while (Right (x) != nil)
    x = Right (x);
  return Closure (x);
}

/*
 * Get a StrTreeNode with the arg key.
 */

static StrTreeNode
mknode (char *key, void* closure)
{
  StrTreeNode node;

  node = (StrTreeNode) getmem (sizeof (STNode));
  /*bzero (node, sizeof (STNode));*/
  Parent (node) = Left (node) = Right (node) = nil;
  Key (node) = key;
  Closure (node) = closure;

  return node;
}

/*
 * Left rotation
 */

static void
left_rotate (StrTreeNode *tree, StrTreeNode x)
{
  StrTreeNode y;

  if (Right (x) != nil)
    {
      y = Right (x);
      Right (x) = Left (y);
      if (Left (y) != nil)
	Parent (Left (y)) = x;
      Parent (y) = Parent (x);

      if (Parent (x) == nil)
	*tree = y;
      else if (x == Left (Parent (x)))
	Left (Parent (x)) = y;
      else
	Right (Parent (x)) = y;
      Left (y) = x;
      Parent (x) = y;
    }
}

/*
 * Right rotation
 */

static void
right_rotate (StrTreeNode *tree, StrTreeNode y)
{
  StrTreeNode x;

  if (Left (y) != nil)
    {
      x = Left (y);
      Left (y) = Right (x);
      if (Right (x) != nil)
	Parent (Right (x)) = y;
      Parent (x) = Parent (y);

      if (Parent (y) == nil)
	*tree = y;
      else if (y == Right (Parent (y)))
	Right (Parent (y)) = x;
      else
	Left (Parent (y)) = x;
      Right (x) = y;
      Parent (y) = x;
    }
}

/*
 * Plain binary tree insertion.
 */

static void
insert_node (StrTreeNode *tree, StrTreeNode z)
{
  StrTreeNode x, y;

  y = nil;
  x = *tree;

  while (x != nil)
    {
      y = x;
      if (strcmp (z->key, x->key) < 0)
	x = Left (x);
      else /* 同じキーの時も右につなげる */
	x = Right (x);
    }
  Parent (z) = y;

  if (y == nil)
    *tree = z;
  else if (strcmp (z->key, y->key) < 0)
    Left (y) = z;
  else
    Right (y) = z;
}

/*
 * Public insertion routine
 */

StrTreeNode
STreeInsert (StrTreeNode *tree, char *key, void* closure)
{
  StrTreeNode x, y, z;

  z = x = mknode (key, closure);
  insert_node (tree, x);
  Color (x) = Red;

  while (x != *tree && Parent (x) && Color (Parent (x)) == Red)
    {
      if (Parent (x) == Left (Parent (Parent (x))))
	{
	  y = Right (Parent (Parent (x)));
	  if (y && Color (y) == Red)
	    {
	      Color (Parent (x)) = Black;
	      Color (y) = Black;
	      Color (Parent (Parent (x))) = Red;
	      x = Parent (Parent (x));
	    }
	  else
	    {
	      if (x == Right (Parent (x)))
		{
		  x = Parent (x);
		  left_rotate (tree, x);
		}
	      Color (Parent (x)) = Black;
	      Color (Parent (Parent (x))) = Red;
	      right_rotate (tree, Parent (Parent (x)));
	    }
	}
      else
	{
	  y = Left (Parent (Parent (x)));
	  if (y && Color (y) == Red)
	    {
	      Color (Parent (x)) = Black;
	      Color (y) = Black;
	      Color (Parent (Parent (x))) = Red;
	      x = Parent (Parent (x));
	    }
	  else
	    {
	      if (x == Left (Parent (x)))
		{
		  x = Parent (x);
		  left_rotate (tree, x);
		}
	      Color (Parent (x)) = Black;
	      Color (Parent (Parent (x))) = Red;
	      right_rotate (tree, Parent (Parent (x)));
	    }
	}
    }
  Color (*tree) = Black;
  return z;
}


/*
 * Search arg key. Returns closure
 */

void*
STreeSearch (StrTreeNode root, char* key)
{
  register StrTreeNode x;
  register int r;

  x = root;
  while (x != nil && (r = strcmp (key, x->key)) != 0)
    {
      if (r < 0)
	x = Left (x);
      else
	x = Right (x);
    }
  return  x == nil ? NULL : Closure (x);
}

/*
 * Search arg key. Returns closure
 */

void*
STreeSearchByUserComp (StrTreeNode root, char* key, int (*comp)(char*, char*))
{
  register StrTreeNode x;

  x = root;
  while (x != nil && (*comp)(key, x->key) != 0)
    {
      if ((*comp)(key, x->key) < 0)
	x = Left (x);
      else
	x = Right (x);
    }
  return  x == nil ? NULL : Closure (x);
}

/*
 * Search arg key. Returns node
 */

StrTreeNode
STreeSearchNode (StrTreeNode root, char* key)
{
  register StrTreeNode x;
  register int r;

  x = root;
  while (x != nil && (r = strcmp (key, x->key)) != 0)
    {
      if (r < 0)
	x = Left (x);
      else
	x = Right (x);
    }
  return  x == nil ? NULL : x;
}

/*
 * Continuesly search arg key. Returns node
 *
 * Usage:
 *  StrTreeNode node;
 *
 *  node = STreeSearchNext (root, key); 一回目
 *  node = STreeSearchNext (node, key); 二回目（前に返ったノードポインタを渡す）
 *  node = STreeSearchNext (node, key); 三回目（前に返ったノードポインタを渡す）
 *  node = STreeSearchNext (node, key); 四回目（前に返ったノードポインタを渡す）
 *   ...
 */

StrTreeNode
STreeSearchNext (StrTreeNode node, char* key)
{
  register StrTreeNode x;
  register int r;

  x = node;
  if (strcmp (key, x->key) == 0) /* 自身はスキップする */
    x = Right (x);

  while (x != nil && (r = strcmp (key, x->key)) != 0)
    {
      if (r < 0)
	x = Left (x);
      else
	x = Right (x);
    }
  return  x == nil ? NULL : x;
}


/* pp 248 */
static StrTreeNode
tree_minimum (StrTreeNode x)
{
  while (Left (x) != nil)
    x = Left (x);
  return x;
}

/* pp 248 */
static StrTreeNode
tree_maximum (register StrTreeNode x)
{
  while (Right (x) != nil)
    x = Right (x);
  return x;
}

/* pp 249 */
static StrTreeNode
tree_successor (register StrTreeNode x)
{
  register StrTreeNode y;

  if (Right (x) != nil)
    return tree_minimum (Right (x));
  y = Parent (x);
  while (y != nil && x == Right (y))
    {
      x = y;
      y = Parent (y);
    }
  return y;
}

#define DOIT

static void
delete_fixup (StrTreeNode* tree, register StrTreeNode x)
{
  register StrTreeNode w;

  while (x != *tree && Color (x) == Black)
    {
      if (x == Left (Parent (x)))
	{
	  w = Right (Parent (x));
#ifdef DOIT
	  if (w == nil) break;  /* ???? */
#endif
	  if (Color (w) == Red)
	    {
	      Color (w) = Black;
	      Color (Parent (x)) = Red;
	      left_rotate (tree, Parent (x)); 
	      w = Right (Parent (x));
	    }
#ifdef DOIT
	  if (w == nil) break;  /* ???? */
#endif
	  if (Color (Left (w)) == Black && Color (Right (w)))
	    {
	      Color (w) = Red;
	      x = Parent (x);
	    }
	  else
	    {
	      if (Color (Right (w)) == Black) /* L12 */
		{
		  Color (Left (w)) = Black;
		  Color (w) = Red;
		  right_rotate (tree, w);
		  w = Right (Parent (x));
		}
	      Color (w) = Color (Parent (x)); /* L17 */
	      Color (Parent (x)) = Black;
	      Color (Right (w)) = Black;
	      left_rotate (tree, Parent (x));
	      x = *tree;
	    }
	}
      else
	{
	  w = Left (Parent (x));
#ifdef DOIT
	  if (w == nil) break;  /* ???? */
#endif
	  if (Color (w) == Red)
	    {
	      Color (w) = Black;
	      Color (Parent (x)) = Red;
	      left_rotate (tree, Parent (x)); 
	      w = Left (Parent (x));
	    }
#ifdef DOIT
	  if (w == nil) break;  /* ???? */
#endif
	  if (Color (Right (w)) == Black && Color (Left (w)))
	    {
	      Color (w) = Red;
	      x = Parent (x);
	    }
	  else
	    {
	      if (Color (Left (w)) == Black) /* L12 */
		{
		  Color (Right (w)) = Black;
		  Color (w) = Red;
		  right_rotate (tree, w);
		  w = Left (Parent (x));
		}
	      Color (w) = Color (Parent (x)); /* L17 */
	      Color (Parent (x)) = Black;
	      Color (Left (w)) = Black;
	      left_rotate (tree, Parent (x));
	      x = *tree;
	    }
	} 
    }
  Color (x) = Black;
}

/* 
 * Delete node z from tree.
 */

static StrTreeNode
delete_node (StrTreeNode* tree, register StrTreeNode  z)
{
  register StrTreeNode x, y;

  if (Left (z) == nil || Right (z) == nil)
    y = z;
  else
    y = tree_successor (z);
  
  if (Left (y) != nil)
    x = Left (y);
  else
    x = Right (y);

  Parent (x) = Parent (y);
  
  if (Parent (y) == nil)
    *tree = x;
  else
    {
      if (y == Left (Parent (y)))  /* L10 */
	Left (Parent (y)) = x;
      else
	Right (Parent (y)) = x;
    }
  if (y != z)
    {
      Key (z) = Key (y);  /* もしノードが別の値を持つならそれらもコピー*/
      Closure (z) = Closure (y);
    }
  if (Color (y) == Black)
    delete_fixup (tree, x);
  return y;
}

/*
 * ノードを１つ解放する. ユーザが解放するためにクロージャーを返す
 */

void*
STreeDelete (StrTreeNode* tree, char* key)
{
  StrTreeNode node;
  void* ret = NULL;

  if ((node = STreeSearchNode (*tree, key)))
    {
      ret = Closure (node);
      (void) delete_node (tree, node);
    }
  return ret;
}

/*
 * Apply/Dealloc関数はネストしてコールすることができるようにするために
 * ユーザ関数のスタックを作った。
 */
static int  nest_ptr = 0;
static int (*nest_stack[MAX_NEST])(void*);

static void
tree_dealloc (StrTreeNode n)
{
  if (n != NULL && n != nil)
    {
      tree_dealloc (Left (n));
      tree_dealloc (Right (n));
      (*nest_stack[nest_ptr-1]) (n);
      memfree ((char*)n);
    }
}

/*
 * Dealloc tree and closure
 */

void
STreeDealloc (StrTreeNode tree, void (*fp)(StrTreeNode))
{
  if (tree)
    {
      nest_stack[nest_ptr++] = (int(*)(void*))fp;
      tree_dealloc (tree);
      nest_ptr --;
    }
}

static void
tree_only_dealloc (StrTreeNode n)
{
  if (n != nil)
    {
      tree_only_dealloc (Left (n));
      tree_only_dealloc (Right (n));
      memfree ((char*)n);
    }
}

/*
 * Dealloc tree
 */
void
STreeDeallocTree (StrTreeNode tree)
{
  if (tree)
    tree_only_dealloc (tree);
}


static void
traverse_apply_closure (StrTreeNode r)
{
  if (r != nil)
    {
      traverse_apply_closure (Left (r));
      (*nest_stack[nest_ptr-1]) (Closure (r));
      traverse_apply_closure (Right (r));
    }
}

/*
 * Apply whatever function to closure.
 */

void
STreeApply (StrTreeNode root, void (*app)(void*))
{
  if (root)
    {
      nest_stack[nest_ptr++] = (int(*)(void*)) app;
      traverse_apply_closure (root);
      nest_ptr --;
    }
}

static void
traverse_apply_node (StrTreeNode r)
{
  if (r != nil)
    {
      traverse_apply_node (Left (r));
      (*nest_stack[nest_ptr-1]) (r);
      traverse_apply_node (Right (r));
    }
}

/*
 * Apply whatever function to node
 */

void
STreeApplyNode (StrTreeNode root, void (*app)(StrTreeNode))
{
  if (root)
    {
      nest_stack[nest_ptr++] = (int(*)(void*)) app;
      traverse_apply_node (root);
      nest_ptr --;
    }
}

static int done;

static void
traverse_cond (StrTreeNode r)
{
  if (!done && r != nil)
    {
      traverse_cond (Left (r));
      if ((*nest_stack[nest_ptr-1])(Closure (r)))
	{
	  done = True;
	  return;
	}
      traverse_cond (Right (r));
    }
}

/*
 * Apply whatever function to closure.
 */

int
STreeApplyCond (StrTreeNode root, int (*app)(void*))
{
  done = False;
  if (root)
    {
      nest_stack[nest_ptr++] = app;
      traverse_cond (root);
      nest_ptr--;
    }
  return done;
}

/* Print keys
 */
void STreePrint (StrTreeNode r)
{
  if (r != nil)
    {
      STreePrint (Left (r));
      printf ("\'%s\'\n", r->key);
      STreePrint (Right (r));
    }
}

