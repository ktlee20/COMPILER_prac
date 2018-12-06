CC = gcc

CFLAGS =

OBJS_FLEX = main.o util.o lex.yy.o y.tab.o symtab.o analyze.o

#by flex
cminus_flex: $(OBJS_FLEX)
	$(CC) $(CFLAGS) $(OBJS_FLEX) -o cminus_flex -lfl

main.o: main.c globals.h util.h scan.h analyze.h symtab.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c

lex.yy.o: cminus.l scan.h util.h globals.h
	flex cminus.l
	$(CC) $(CFLAGS) -c lex.yy.c -lfl

y.tab.o: util.h globals.h y.tab.h cminus.y y.tab.h
	$(CC) $(CFLAGS) -c y.tab.c -lfl

symbal.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	$(CC) $(CFLAGS) -c analyze.c



clean:
	-rm cminus_flex
	-rm $(OBJS_FLEX)
	-rm lex.yy.c
