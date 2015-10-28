/*
 * strtree.h
 *
 * Red-Black Tree char* as key version
 *
 * Reference: Introduction to ALGORITHMS from MIT Press
 *
 * You must start with a dummy node that contains the minimum
 * key value.
 *
 * This version has a char pointer as the key.
 *
 */
#ifndef Strtree_h
#define Strtree_h

#define MAX_NEST   256

typedef struct _StrTreeNode {
	struct _StrTreeNode* parent;
	struct _StrTreeNode* left;
	struct _StrTreeNode* right;
	unsigned char color;
	unsigned char pad;   /* to keep even boundary */
	char*   key;         /* key string (null terminated) */
	void*   closure;     /* anything that related to the key */
} STNode, *StrTreeNode;



#define	RED	0
#define Red	0
#define BLACK	1
#define Black	1

#define Right(n)  ((n)->right)
#define Left(n)	  ((n)->left)
#define Parent(n) ((n)->parent)
#define Color(n)  ((n)->color)
#define Key(n)    ((n)->key)
#define Closure(n) ((n)->closure)

#ifndef NULL
#define NULL (void*)0
#endif
#ifndef True
#define True  1
#define False 0
#endif

typedef void (*STreeApplyProc)(void*);
typedef void (*STreeApplyNodeProc)(StrTreeNode);
typedef int  (*STreeApplyCondProc)(void*);
typedef int  (*STreeUserCompProc)(char* , char*);

void		STreeInit (StrTreeNode* root);
StrTreeNode	STreeInsert (StrTreeNode *root, char* key, void* closure);
void*		STreeSearch (StrTreeNode root, char* key);
void*		STreeSearchByUserComp (StrTreeNode root, char* key, STreeUserCompProc p);
StrTreeNode	STreeSearchNode (StrTreeNode root, char* key);
StrTreeNode	STreeSearchNext (StrTreeNode node, char* key);
void*   	STreeDelete (StrTreeNode* root, char* key);
void		STreeDealloc (StrTreeNode root, void (*fp)(StrTreeNode));
void		STreeDeallocTree (StrTreeNode tree);
void		STreeApply (StrTreeNode root, void (*app)(void*));
void		STreeApplyNode (StrTreeNode root, void (*app)(StrTreeNode));
int		STreeApplyCond (StrTreeNode root, int (*app)(void*));
void*		STreeSample (StrTreeNode root);
void            STreePrint (StrTreeNode r);


#endif



