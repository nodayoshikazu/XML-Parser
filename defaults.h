/*
 *
 *    defaults.h
 *
 *
 *
 *
 *   08/14/01
 *
 */
#ifndef DEFAULTS_H
#define DEFAULTS_H


#define FILE_URI_HEADER1  "FILE://"
#define FILE_URI_HEADER2  "file://"
#define FILE_URI_HEADERL  7


/* �����element�κ���ͥ��ȿ��Ǥ褤 */
#define STACKSIZE      128


/* URI��Ĺ�� */
#define URI_LENGTH     256

/* setset�ǵ��������祻�åȿ� */
#define MAXSETS        1024

/* intset MAX value */
#define MAX_BITS       256

/* DFA�ط� */
#define MAX_LEAVES     1024
#define MAX_GOTOS      1024

/* XML name�ǵ������κ����Ĺ�� */
#define MAXNAMELEN     256

/* XML entity���֤����������ե���󥹤κ����Ĺ�� */
#define MAX_REFERENCE_LENGTH  256

/* ������Ȥνи������å��������(getNextStateOnSymbol) */
#define ACCEPT         1
#define NOT_ACCEPT     0

#endif

