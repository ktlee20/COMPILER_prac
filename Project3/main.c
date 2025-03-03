#include "globals.h"

#define NO_PARSE FALSE

#define NO_ANALYZE FALSE

#include "util.h"

#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include"analyze.h"
#endif
#endif

int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = TRUE;
int TraceCode = FALSE;

int Error = FALSE;

main(int argc, char * argv [])
{
	TreeNode * syntaxTree;
	char pgm[120];

	if(argc != 2)
	{
		fprintf(stderr, "usage : %s <filename>\n",argv[0]);
		exit(1);
	}
	strcpy(pgm,argv[1]);

	if(strchr(pgm,'.') == NULL)
		strcat(pgm,".cm");

	source = fopen(pgm,"r");

	if(source == NULL)
	{
		fprintf(stderr,"File %s not found\n",pgm);
		exit(1);
	}

	listing = stdout;
	fprintf(listing, "\nCMINUS COMPILATION: %s\n",pgm);

#if NO_PARSE
	while(getToken()!=ENDFILE);
#else
	//call parse
	syntaxTree = parse();
	if(TraceParse)
	{
		fprintf(listing,"\nSyntax tree:\n");
		printTree(syntaxTree);
	}
#if !NO_ANALYZE
	if(! Error)
	{
		if(TraceAnalyze) fprintf(listing, "\nBuilding Symbol Table...\n");
		buildSymtab(syntaxTree);
		if(TraceAnalyze) fprintf(listing, "\nChecking Types...\n");
		typeCheck(syntaxTree);
		if(TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
	}
#endif
#endif
	fclose(source);
	return 0;
}
