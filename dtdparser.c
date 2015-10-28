/*
 *
 *  dtdparser.c
 *
 *  DTDファイルパスがあるかどうか見て、あればパースする。
 *
 *  08/06/01 FILE URIからパスにコンバート出来る。
 *  08/15/01 Win32 port
 *  10/18/01 Bug fix
 */
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/param.h>
#endif
#include "defaults.h"
#include "debug.h"

#ifdef WIN32
# define MAXPATHLEN 512
#endif

/* public */
int          dtd_parsing_result = 0;

static char* SYSTEM         = "SYSTEM";
static int   SYSTEM_LENGTH  = 6;
static char* DOCTYPE        = "DOCTYPE";
static int   DOCTYPE_LENGTH = 7;
static int   lines = 0;


/*
 * ファイルから、引数の文字列(長さも渡す)を探す。
 */

static int lookup_string (FILE* file, char* string, int length)
{
  int ch, match = 0;
  int found = 0;

  while((ch = fgetc (file)) != EOF) {
    if (ch == '\n')
      lines++;
    if (match == 0) {
      if (ch == string[0])
	match = 1;
      else
	match = 0;
    }
    else {
      if (ch == string[match])
	match++;
      else
	match = 0;
      if (match == length) {
	found = 1;
	break;
      }
    }
  }
  return found;
}

/*
 * FILE URIをシステムパスに変換
 */

static char* convert_file_uri_to_system_path (char* uri)
{
  char work[MAXPATHLEN];

  if (strncmp (FILE_URI_HEADER1, uri, FILE_URI_HEADERL) == 0 
      || strncmp (FILE_URI_HEADER2, uri, FILE_URI_HEADERL) == 0) {
    int len = strlen (uri) - FILE_URI_HEADERL;
    char* p;
    memcpy (work, uri+FILE_URI_HEADERL, len);
    work[len] = '\0';
#ifdef WIN32
    p = work;
    while (*p) {
      if (*p == '/')
	*p = '\\';
      p++;
    }
#endif
    memcpy (uri, work, len);	/* もどすだけ */
    uri[len] = '\0';
  }
  return uri;
}

/*
 * XMLファイルからDTDパスを検索
 */

char* lookupDTDPath (FILE* file)
{
  static char dtdpath[MAXPATHLEN];
  int ch, match = 0;
  int line = 0;

  lines = 0;			/* fixed bug 10/18/01! */
  if (lookup_string (file, DOCTYPE, DOCTYPE_LENGTH)) {
    if (5 < lines)
      return NULL;		/* not found */
    if (lookup_string (file, SYSTEM, SYSTEM_LENGTH)) {
      char* p = dtdpath;
      while ((ch = fgetc (file)) != '"')
	;
      while ((ch = fgetc (file)) != '"')
	*p++ = ch;
      *p = '\0';
    }
    convert_file_uri_to_system_path (dtdpath);
    return dtdpath;
  }
  return NULL;
}

/*
 * DTDをパースする
 *  
 */

int parseDTD (char* dtdpath) {
  FILE* f;
  int  r = -1;

  ENTER ("parseDTD");
  if (f = fopen (dtdpath, "r")) {
    r = parseXml (f);  /* DTDをパースする */
    fclose (f);
  }
  else
    warn3 ("Specified DTD file \"", dtdpath, "\" does not exist.");
  LEAVE ("parseDTD");
  return r;
}

/*
main (int ac, char* av[])
{
  dtdparser (av[1]);
}
*/
