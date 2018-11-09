#include "globals.h"

#define NO_PARSE TRUE

#include "util.h"

#if NO_PARSE
#include "scan.h"
#endif

/**
 *
 update entity
 1. If "read or write ..." are identifier , this tiny scanner couldn't know them. So, except the scanner, I remove all.
 2. Change Makefile.
 */


int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

int EchoSource = TRUE;
int TraceScan = TRUE;

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
#endif

	fclose(source);
	return 0;
}
