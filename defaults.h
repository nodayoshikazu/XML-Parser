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


/* これはelementの最大ネスト数でよい */
#define STACKSIZE      128


/* URIの長さ */
#define URI_LENGTH     256

/* setsetで許される最大セット数 */
#define MAXSETS        1024

/* intset MAX value */
#define MAX_BITS       256

/* DFA関係 */
#define MAX_LEAVES     1024
#define MAX_GOTOS      1024

/* XML nameで許されるの最大の長さ */
#define MAXNAMELEN     256

/* XML entityで置き換えられるリファレンスの最大の長さ */
#define MAX_REFERENCE_LENGTH  256

/* エレメントの出現チェックの戻り値(getNextStateOnSymbol) */
#define ACCEPT         1
#define NOT_ACCEPT     0

#endif

