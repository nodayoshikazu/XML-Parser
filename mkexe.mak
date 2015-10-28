#
#  makefile for sax
#
OS = solaris
PLATFORM = i386
THREADS = native_threads
JDK_HOME = /usr/j2se
JDK_INCLUDES = -I${JDK_HOME}/include -I${JDK_HOME}/include/${OS}
JAVALIB = java

CC = gcc
LEX = lex
LEXOPT = -v -e
YACC = yacc
YACCOPT = -v

INCLUDES = 
#INCLUDES = ${JDK_INCLUDES}
LIB_PATH = ${JDK_HOME}/lib/${PLATFORM}/${THREADS}
#LIB_PATH = ${JDK_HOME}/lib
#CCFLAGS = -g -DDEBUG -DYYDEBUG
CCFLAGS = -g 

.c.o:
	${CC} ${CCFLAGS} ${INCLUDES} -c $<

.java.class:
	javac $<

OBJS = xml.lex.o xml.tab.o strtree.o 
SRCS = xml.lex.c xml.tab.c strtree.c 
OBJS2 = 
SRCS2 = 
OBJS1 = main.o symtab.o attrib.o saxhandler.o saxparser.o dfa.o intset.o setset.o mem.o dtdparser.o stack.o
PROG = sax


all:: xml.lex.c xml.tab.c ${OBJS} ${PROG}

sax: $(OBJS) $(OBJS1) $(OBJS2)
	$(CC) -g -o sax $(OBJS1) $(OBJS) $(OBJS2) -ll

xpp: $(OBJS2)
	$(CC) -g -o xpp $(OBJS2)

xml.lex.c : xml.l
	${LEX} ${LEXOPT} xml.l
	mv lex.yy.c xml.lex.c

xml.tab.c : xml.y
	${YACC} ${YACCOPT} -b xml -d xml.y

clean	:
	$(RM) $(PROG) *.o *~ core *.class lex.yy.c ooo *.output 

