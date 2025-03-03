/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
   { START, INEQ, INCOMMENT, INNUM, INID, DONE, INLT, INGT, INNE, INOVER, INCOMMENT_ }
   StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{ 
	/*current position isn't less than buffer size
	  case1: line end-> go to next line;
	  case2: source file end -> return EOF
	  */
	if (!(linepos < bufsize))
  	{ 
		lineno++;
		//case1-> read nextline.
    	if (fgets(lineBuf,BUFLEN-1,source))
    	{ 
			//If EchoSource is true, print to monitor.
			if (EchoSource) 
				fprintf(listing,"%4d: %s",lineno,lineBuf);
			//bufsize has length of line
      		bufsize = strlen(lineBuf);
      		linepos = 0;
			//return next character
      		return lineBuf[linepos++];
    	}
		//case2-> set End of file flag to TRUE
    	else
    	{ 
			//return EOF to show End of file
			EOF_flag = TRUE;
      	return EOF;
    	}
  	}
	//curruent position is less than buffer size-> return next character.
  	else 
		return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */

//if it isn't EOF, backtracks one character.
static void ungetNextChar(void)
{ 
	if (!EOF_flag) 
		linepos-- ;
}

/* lookup table of reserved words */
static struct
    { char* str;
      TokenType tok;
    } reservedWords[MAXRESERVED]
	= {{"if",IF},{"else",ELSE},{"while",WHILE},{"return",RETURN},{"int",INT},{"void",VOID},
	   /* discarded  {"then",THEN}, {"end",END},{"repeat",REPEAT},{"until",UNTIL},{"read",READ},{"write",WRITE} */
	  };
/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup (char * s)
{ int i;
  for (i=0;i<MAXRESERVED;i++)
    if (!strcmp(s,reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the 
 * next token in source file
 */
TokenType getToken(void)
{  /* index for storing into tokenString */
   int tokenStringIndex = 0;
   /* holds current token to be returned */
   TokenType currentToken;
   /* current state - always begins at START */
   StateType state = START;
   /* flag to indicate save to tokenString */
   int save;
   while (state != DONE)
   { 
	 //read character to lexical analyzing
	 int c = getNextChar();
	 //save is default
     save = TRUE;
	 
     switch (state)
     {
		 /**
		   case START :
			1. start of source
			2. starting with comments
		   */
		 case START:
		 /**
		   start start
		   input
		   number - state->INNUM
           alphabet - state->INID
		   : -  state=->INASSIGN
		   space - save=FALSE
		   { - save = FALSE, state->INCOMMNET
		   rest - end
		   */
         if (isdigit(c))
           state = INNUM;
         else if (isalpha(c))
           state = INID;
         /*else if (c == ':')
           state = INASSIGN;*/
         else if ((c == ' ') || (c == '\t') || (c == '\n'))
           save = FALSE;
         else if (c == '/')
         { 
		   save = FALSE;
           state = INOVER;
         }
		 else if (c == '=')
		 {
		   state = INEQ;
		 }
		 else if (c == '!')
		 {
		   save = FALSE;
		   state = INNE;
		 }
		 else if (c == '<')
		 {
		   state = INLT;
		 }
		 else if (c == '>')
		 {
		   state = INGT;
		 }
         else
         { 
           //end
		   state = DONE;
           switch (c)
           { case EOF:
               save = FALSE;
               currentToken = ENDFILE;
               break;
             /*case '=':
               currentToken = EQ;
               break;
			   */
             /*
			   case '<':
               currentToken = LT;
               break;
			   */
             case '+':
               currentToken = PLUS;
               break;
             case '-':
               currentToken = MINUS;
               break;
             case '*':
               currentToken = TIMES;
               break;
             /*case '/':
               currentToken = OVER;
               break;
			   */
             case '(':
               currentToken = LPAREN;
               break;
             case ')':
               currentToken = RPAREN;
               break;
			 case '{':
			   currentToken = LCURLY;
			   break;
			 case '}':
			   currentToken = RCURLY;
			   break;
			 case '[':
			   currentToken = LBRACE;
			   break;
			 case ']':
			   currentToken = RBRACE;
			   break;
             case ';':
               currentToken = SEMI;
               break;
			 case ',':
			   currentToken = COMMA;
			   break;
             default:
               currentToken = ERROR;
               break;
           }
         }
         break;
	   case INOVER:
		 save = FALSE;
		 if(c == '*')
			 state = INCOMMENT;
		 else
		 {
			ungetNextChar();
     		if (tokenStringIndex <= MAXTOKENLEN)
       				tokenString[tokenStringIndex++] = '/';
			state = DONE;
			currentToken = OVER;
		 }
		 break;
	   case INLT:
		 if(c == '=')
		 {
			 state = DONE;
			 currentToken = LE;
		 }
		 else
		 {
			 ungetNextChar();
			 state = DONE;
			 currentToken = LT;
		 }
		 break;
	   case INGT:
		 if(c == '=')
		 {
			 state = DONE;
			 currentToken = GE;
		 }
		 else
		 {
			 ungetNextChar();
			 state = DONE;
			 currentToken = GT;
		 }
		 break;
       case INCOMMENT:
		 save = FALSE;
		 if ( c == '*')
			 state = INCOMMENT_;
		 else if ( c == EOF)
		 {
			 state = DONE;
			 currentToken = ENDFILE;
		 }
		 break;
         /*save = FALSE;
         if (c == EOF)
         { 
		   state = DONE;
           currentToken = ENDFILE;
         }
         else if (c == '}') 
			 state = START;
         break;*/
	   case INCOMMENT_:
		 save = FALSE;
		 if( c == '/')
			 state = START;
		 else if( c == '*')
			 state = INCOMMENT_;
		 else if( c == EOF)
		 {
			state = DONE;
			currentToken = ENDFILE;
		 }
		 else
			state = INCOMMENT;
		 break;
       /*case INASSIGN:
         state = DONE;
         if (c == '=')
           currentToken = ASSIGN;
         else
         { //backup in the input 
           ungetNextChar();
           save = FALSE;
           currentToken = ERROR;
         }
         break; */
       case INNUM:
         if (!isdigit(c))
         { /* backup in the input */
		   //if number's next character is not number -> DONE
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = NUM;
         }
         break;
       case INID:
         if (!isalpha(c))
         { /* backup in the input */
		   //if alphabet's next is not alphabet -> DONE
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = ID;
         }
         break;
	   case INEQ:
		 if(c == '=')
		 {
			 currentToken = EQ;
			 state = DONE;
		 }
		 else
		 {
			 ungetNextChar();
			 save = FALSE;
			 state = DONE;
			 currentToken = ASSIGN;
		 }
		 break;
	   case INNE:
		 if(c == '=')
		 {
     		if (tokenStringIndex <= MAXTOKENLEN)
       				tokenString[tokenStringIndex++] = '!';
			 currentToken = NE;
			 state = DONE;
		 }
		 else
		 {
			ungetNextChar();
			state = DONE;
			currentToken = ERROR;
		 }
		 break;
       case DONE:
       default: /* should never happen */
         fprintf(listing,"Scanner Bug: state= %d\n",state);
         state = DONE;
         currentToken = ERROR;
         break;
     }
     if ((save) && (tokenStringIndex <= MAXTOKENLEN))
       tokenString[tokenStringIndex++] = (char) c;
     if (state == DONE)
     { 
	   tokenString[tokenStringIndex] = '\0';
	   //if current Token is identifier or reserved word, save it to Loopkuptable
       if (currentToken == ID)
         currentToken = reservedLookup(tokenString);
     }
   }
   // if TraceScan is true, print to monitor
   if (TraceScan) {
     fprintf(listing,"\t%d: ",lineno);
     printToken(currentToken,tokenString);
   }

   //return information of currentToken
   return currentToken;
} /* end getToken */

