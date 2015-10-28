#
#  makefile for xml-parser
#
#
#
#
#
OS = solaris
PLATFORM = sparc
CC = cc
LEX = lex
YACC = yacc
AR = /usr/ccs/bin/ar cq

INCLUDES = 
LIB_PATH = 
LIBS     = -ll -lm -ldl -lpthread -lposix4 -lsocket -lnsl
DEFS     = -D_REENTRANT -DEXPORTED_SYSTEM_ADMIN_API=1 -DSTATIC_APPLIB -DMEMCHK=1

INCLUDES = -I. -I../include
#LEXOPT   = -v -e
LEXOPT   = -e 
YACCOPT  = -v

# gcc
#CCOPT    = -g -fpic
LDOPT    = -shared

# cc
CCOPT    = -O -mt -Kpic
#LDOPT    = -G

#gcc/cc
CCFLAGS = $(CCOPT) $(DEFS) $(INCLUDES)

.c.o:
	${CC} ${CCFLAGS} ${INCLUDES} -c $<
.cpp.o:
	${CC} ${CCFLAGS} ${INCLUDES} -c $<

SRCS = attrib.c dfa.c dtdparser.c intset.c mem.c saxhandler.c saxparser.c setset.c \
       stack.c strtree.c symtab.c xml.lex.c xml.tab.c
OBJS = attrib.o dfa.o dtdparser.o intset.o mem.o saxhandler.o saxparser.o setset.o \
       stack.o strtree.o symtab.o xml.lex.o xml.tab.o
PROG = libxmlp.a

all:: xml.tab.c xml.lex.c $(OBJS) $(PROG)

xml.lex.c : xml.l
	${LEX} ${LEXOPT} xml.l
	sed -e s/YY/XX/g -e s/yy/xx/g lex.yy.c > xml.lex.c
	$(RM) -f lex.yy.c

xml.tab.c : xml.y
	${YACC} ${YACCOPT} -b xml -d xml.y
	sed -e s/YY/XX/g -e s/yy/xx/g xml.tab.c > c
	sed -e s/YY/XX/g -e s/yy/xx/g xml.tab.h > h
	mv c xml.tab.c
	mv h xml.tab.h

libxmlp.a: $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

libxmlp.so: $(OBJS)
	$(RM) $@
	$(CC) $(LDOPT) -o $@ $(OBJS)

##
clean	:
	$(RM) $(OBJS) ${PROG} *.o *~ core *.class a.out *.a *.so *.msg




