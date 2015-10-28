/*
 *
 *   attrib.c
 *
 *
 *
 *
 *  08/10/2001
 */
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "strtree.h"
#include "mem.h"
#include "attrib.h"
#include "debug.h"


/*
 * Add a name to Nink list
 */

static void addNameToFixed (Fixed* fixed, char* name)
{
  Nink* new_nink = (Nink*) getmem (sizeof (Nink));

  new_nink->name = name;
  new_nink->next = NULL;
  if (fixed->fixed_candidates != NULL)
    fixed->last_candidate->next = new_nink;
  else
    fixed->fixed_candidates = new_nink;
  fixed->last_candidate = new_nink;      
  fixed->n_fixed++;
}

/*
 * free Nink list
 */

static void releaseNinks (Fixed* fixed)
{
  if (fixed->fixed_candidates != NULL) {
    Nink* nink = fixed->fixed_candidates;
    while (nink) {
      Nink* tnink = nink->next;
      memfree ((void*)nink);
      nink = tnink;
    }
  }
}

/*
 * Allocate Fixed struct
 */

Fixed* newFixedStruct (void)
{
  Fixed* fixed = (Fixed*) getmem (sizeof (Fixed)); /* yuck! */
  fixed->n_fixed = 0;
  fixed->fixed_candidates = NULL;
  fixed->last_candidate = NULL;
  return fixed;
}

/*
 * Fixed��¤�Τ����
 */

void releaseFixedStruct (Fixed* fixed)
{
  releaseNinks (fixed);
  memfree ((void*) fixed);
}

/*
 * ������Attribute��¤�Τ����
 */

Attribute* newAttributeStruct (char* name)
{
  Attribute* attr = (Attribute*) getmem (sizeof (Attribute));
  attr->type = 0;
  attr->name = name;		/* attribute name */
  attr->fixed = NULL;
  attr->next = NULL;		/* next link */
  return attr;
}

/*
 * type = AT_CDATA|AT_ID|AT_IDREF...
 * attrib = (AX_REQUIRED|AX_IMPLIED|AX_FIXED) 
 */

Attribute* setAttributeType (Attribute* attr, int type, int attrib)
{
  attr->type = type | attrib;
  return attr;
}

/*
 * Attribute��¤�Τ����
 */

void releaseAttribute (Attribute* attr)
{
  if (attr != NULL) {
    while (attr) {
      Attribute* tattr = attr->next;

      if (attr->fixed)
	releaseFixedStruct (attr->fixed);
      memfree ((void*) attr);
      attr = tattr;
    }
  }
}

/*
 * enumeration�����פΥ��ȥ�ӥ塼�Ȥ�enumʸ������ɲä���
 */

Attribute* addEnumNameToAttribute (Attribute* attr, char* name)
{
  if (attr->fixed == NULL)
    attr->fixed = newFixedStruct ();
  addNameToFixed (attr->fixed, name);
  return attr;
}

/*
 * Attribute�Υǥե����ʸ����򥻥åȤ���
 */

Attribute* setDefaultToAttribute (Attribute* attr, char* string)
{
  attr->default_value = string;
  return attr;
}

/*
 * ��Element�����դ���
 */

Element* newElementStruct (char* elem_name)
{
  Element* element = (Element*) getmem (sizeof (Element));

  element->element = elem_name;
  element->elemtype = 0;		/* no type yet */
  element->dfa = NULL;
  element->n_attributes = 0;
  element->attrib_list = NULL;
  return element;
}

/*
 * Element�����
 */

void releaseElementStruct (Element* sa)
{
  releaseDFA (sa->dfa);
  releaseAttribute (sa->attrib_list);
  memfree ((void*) sa);
}

/*
 * Element��DFA�򥻥å�
 */

Element* setDfaToElement (Element* element, Dfa* dfa)
{
  element->elemtype = EL_DFA;
  /*
   * DTD��Ʊ���������̾�ιԤ�����ȸŤ�DFA�˾�񤭤����Τ�
   * �Ť�DFA��������Ƥ�������롣
   */
  if (element->dfa != NULL)
    releaseDFA (element->dfa);
  element->dfa = dfa;
  return element;
}

/*
 * Element�Υ��������ꤷ�ޤ�
 * type = {EL_DFA|EL_PCDATA|EL_EMPTY|EL_ANY}
 */

Element* setTypeToElement (Element* element, unsigned type)
{
  element->elemtype = type;
  return element;
}

/*
 * Element��Attribute���ɲä��ޤ�
 */

Element* addAttributeToElement (Element* element, Attribute* attrib)
{
  if (element->attrib_list != NULL) {
    element->attrib_last->next = attrib;
    element->attrib_last = attrib;      
  } else {
    element->attrib_last = attrib;
    element->attrib_list = attrib;
  }
  element->n_attributes++;
  return element;
}

/*
 * °��̾���顢�б�����Attribute��¤�Τ�����
 */

Attribute* findAttributeFromElement (Element* element, char* name)
{
  if (element) {
    if (element->attrib_list != NULL) {
      Attribute* ap;
      ap = element->attrib_list;
      while (ap) {
	if (strcmp (name, ap->name) == 0)
	  return ap;
	ap = ap->next;
      }
    }
  }
  return NULL;			/* not found */
}

/*
 * °���˴ؤ��ƤޤȤ�ƥ����å������ΤȤ���REQUIRED��°��������äƤ��뤫�ɤ�����
 *  �����å����������֤��ʤ��Τǡ�
 * �����:REQUIRED�ʤΤˡ��ʤ�°���ο�
 *       vec�ˤʤ�°��̾�Υꥹ�Ȥ��֤���
 */

int checkAttributesForRequired (Element* element, StrTreeNode attribRoot, char* vec[])
{
  int required_nonexistent = 0;

  if (element) {
    if (element->attrib_list != NULL) {
      Attribute* ap = element->attrib_list;

      while (ap) {
	if (TypeOfAttribute (ap) & AX_REQUIRED) {
	  /* ��ά�Բ�°�� */
	  StrTreeNode node = STreeSearchNode (attribRoot, ap->name);
	  if (node == NULL)
	    vec[required_nonexistent++] = ap->name; /* �ʤ�°��̾�ɲ� */
	} /* if (TypeOfAttribute (ap) & AX_REQUIRED) */
	ap = ap->next;
      }	/* while */
    }
  }
  return required_nonexistent;
}

/*
 * ENUM�����פ�°���ͤ����������ɤ���Ĵ�٤롣
 * �����: ���顼�������顼���ʤ����0��-1�ʤ�С�°�����Τ��ʤ���
 *  vec[error*2]: °��̾
 *  vec[error*2+1]: �ְ�ä�°����
 *
 */
int checkAttributesForFixed (Element* element, StrTreeNode attribRoot, char* vec[])
{
  int error_fixed =0;
  int k = 0;

  if (element) {
    if (element->attrib_list != NULL) {
      Attribute* ap = element->attrib_list;
      char* value;
      Fixed* fixed;
      int found_enum = 0;

      while (ap) {
	if ((TypeOfAttribute (ap) & AX_REQUIRED)
	    && (ATTypeOfAttribute (ap) == AT_ENUM)) {
	  /* ��ά�Բ�°�� & FIXED */
	  StrTreeNode node = STreeSearchNode (attribRoot, ap->name);
	  if (node == NULL)
	    return -1;		/* �ʤ�! */

	  value = (char*) Closure (node);
	  fixed = FixedOfAttribute (ap);
	  found_enum = 0;

	  if (fixed->fixed_candidates != NULL) {
	    Nink* nink = fixed->fixed_candidates;
	    while (nink) {
	      if (strcmp (NameOfNink (nink), value) == 0)
		found_enum++;
	      nink = NextOfNink (nink);
	    }
	    if (found_enum == 0) {
	      vec[k++] = NameOfAttribute (ap);
	      vec[k++] = value;
	      error_fixed++;
	    }
	  }
	} 
	ap = ap->next;
      }	/* while */
    }
  }
  return error_fixed;
}

/*
 * state transition.
 *  returns accept=1, not accept=0
 */

int getNextStateOnSymbol (Element* parentElement, char* elem)
{
  int accept, i, k=-1, toState=-1;
  Dfa* dfa;

  accept = 0;			/* initiallize to not accepted yet */
  if (parentElement && (dfa = parentElement->dfa)) {
    for (i = 0; i < dfa->n_gotos; i++) {
      if (dfa->gotos[i].fromState == parentElement->stateWork) {
	k = i;
	break;
      }
    }
    if (-1 < k) {
      while (dfa->gotos[k].fromState == parentElement->stateWork) {
	if (strcmp (dfa->gotos[k].input_symbol, elem)==0) {
	  toState = dfa->gotos[k].toState;
	  break;
	}
	k++;
      }
    }
    parentElement->stateWork = toState;
    accept = dfa->gotos[k].accept;
  }
  return accept;
}

/*
 * DFA�������ܽ����
 */
void initStateWorkOfElement (Element* element)
{
  if (element)
    element->stateWork = 0;
}






#ifdef DEBUG
/*=============================================================================
 *  DEBUG..DEBUG..DEBUG..DEBUG..DEBUG..DEBUG..DEBUG..DEBUG..DEBUG..DEBUG..DEBUG
 =============================================================================*/

void printFixed (Fixed* fixed)
{
  Nink* nink;

  printf ("\tFixed{");
  nink = fixed->fixed_candidates;
  while (nink) {
    printf("%s ", nink->name);
    nink = nink->next;
  }
  printf ("}\n");

}

void printAttribute (Attribute* attr)
{
  while (attr) {
    int type = attr->type & 0xff;
    int att  = attr->type & 0xff00;
    printf ("\tname=%s type=%d attrib=%03x\n", attr->name, type, att);
    if (attr->fixed)
      printFixed (attr->fixed);
    attr = attr->next;
  }
}

void printElement (Element* element)
{
  if (element) {
    printf ("Element {\n");
    printf ("\telement=%s type=%04x\n", element->element, element->elemtype);
    if (element->dfa)
      printDfa (element->dfa);
    printf ("\tnumber of attributes=%d\n", element->n_attributes);
    printAttribute (element->attrib_list);
    printf ("}\n");
  }
}


void printAllElementAttributes ()
{
  extern int attribTab;
  extern StrTreeNode getTree(int);
  STreeApply (getTree (attribTab), printElement);
}
#endif
