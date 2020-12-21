# NOTE: ./lex/tiny.l --> cminus.l

CC = gcc
CFLAGS = 

OBJS = y.tab.o main.o util.o lex.yy.o symtab.o code.o analyze.o

all: cminus

cminus: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -lfl

y.tab.o: cminus.l globals.h util.h scan.h parse.h
	yacc -d cminus.y
	$(CC) $(CFLAGS) -c y.tab.c

y.tab.h: y.tab.c

main.o: main.c globals.h y.tab.h util.h scan.h parse.h analyze.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h y.tab.h
	$(CC) $(CFLAGS) -c util.c

lex.yy.c: cminus.l
	flex cminus.l

lex.yy.o: lex.yy.c globals.h y.tab.h util.h scan.h
	$(CC) $(CFLAGS) -c lex.yy.c

symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c globals.h y.tab.h symtab.h analyze.h
	$(CC) $(CFLAGS) -c analyze.c

clean:
	rm -vf $(OBJS) *.o lex.yy.c y.tab.h y.tab.c cminus
