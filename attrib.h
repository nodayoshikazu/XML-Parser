/*
 *
 *   attrib.h
 *
 *
 *
 *
 *  08/10/01
 */
#ifndef ATTRIB_H
#define ATTRIB_H


#include "dfa.h"

/* Attribute type */
#define AT_CDATA    1
#define AT_ID       2
#define AT_IDREF    3
#define AT_IDREFS   4
#define AT_ENTITY   5
#define AT_ENTITIES 6
#define AT_NMTOKEN  7
#define AT_NMTOKENS 8
#define AT_NOTATION 9
#define AT_ENUM     10

/* Attribute of the attribute */
#define AX_REQUIRED  0x100
#define AX_IMPLIED   0x200
#define AX_FIXED     0x400
#define AX_DEFAULT   0x800

/* element type */
#define EL_PCDATA    0x0001	/* 普通のデータですね */
#define EL_EMPTY     0x0002	/* 空エレメント宣言 <.../> */
#define EL_ANY       0x0004	/* なんでもよしなんですが、基本的にはほとんど使われない */
#define EL_DFA       0x0008	/* 子供エレメントのDFAあり */



/* Nink is the name for Name Link 
*/
typedef struct _Nink {
  char*   name;
  struct _Nink *next;
} Nink;

#define NameOfNink(n) ((n)->name)
#define NextOfNink(n) ((n)->next)

/* Fixed Attribute
 */
typedef struct {
  int       n_fixed;		/* number of enum names */
  Nink*     fixed_candidates;	/* enum names list */
  Nink*     last_candidate;
} Fixed;


/* Attribute
 */
typedef struct _Attribute {
  unsigned type;		/* AT_? | (AX_REQUIRED|AX_IMPLIED|AX_FIXED) */
  char*    name;		/* name of the attribute */
  char*    default_value;	/* default value for implied attribute */
  Fixed*   fixed;		/* Has meaning only if type is AT_ENUM */
  struct _Attribute *next;	/* list */
} Attribute;

#define TypeOfAttribute(a)   ((a)->type)
#define ATTypeOfAttribute(a) (((a)->type)&0xFF)
#define NameOfAttribute(a)   ((a)->name)
#define FixedOfAttribute(a)  ((a)->fixed)


/* Element attributes
 */
typedef struct {
  int            stateWork;	/* work variable to sav current dfa transition state  */
  char*          element;	/* name of the element */
  unsigned       elemtype;	/* PCDATA | DFA | EMPTY | ANY */
  Dfa*           dfa;
  int            n_attributes;	/* number of attributes */
  Attribute*     attrib_list;
  Attribute*     attrib_last;
} Element;


#define NameOfElement(s) (s->element)
#define TypeOfElement(s) (s->elemtype)



Fixed*     newFixedStruct (void);
void       releaseFixedStruct (Fixed* fixed);
Attribute* newAttributeStruct (char* name);
Attribute* setAttributeType (Attribute* attr, int type, int attrib);
void       releaseAttribute (Attribute* attr);
Attribute* addEnumNameToAttribute (Attribute* attr, char* name);
Attribute* setDefaultToAttribute (Attribute* attr, char* defname);
Element*   newElementStruct (char* elemname);
void       releaseElementStruct (Element* sa);
Element*   setDfaToElement (Element* syma, Dfa* dfa);
Element*   addAttributeToElement (Element* syma, Attribute* attrib);
Element*   setTypeToElement (Element* syma, unsigned type);
Attribute* findAttributeFromElement (Element* syma, char* name);
int        getNextStateOnSymbol (Element* parent, char* elem);
int        checkAttributesForElement (Element* element, StrTreeNode attribRoot, char* vec[]);
int        checkAttributesForFixed (Element* element, StrTreeNode attribRoot, char* vec[]);

#endif /* ATTRIB_H */
