/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedNumber;
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);

%}

%token ELSE IF INT RETURN VOID WHILE
%token ID NUM
%token PLUS MINUS TIMES OVER LT LE GT GE EQ NE ASSIGN SEMI COMMA LBRACE RBRACE LCURLY RCURLY LPAREN RPAREN
%token ERROR

%% /* Grammar for TINY */
program     : decllist
                 { savedTree = $1;}
            ;
decllist   : decllist decl
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { 
					while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else 
						$$ = $2;
                 }
            | decl  { $$ = $1; }
            ;
decl        : vardecl  { $$ = $1; }
            | fundecl  { $$ = $1; }
            ;
forid		: ID
                 { 
					savedName = copyString(tokenString);
                   savedLineNo = lineno;
                 }
            ;
fornum		: NUM
                 { 
				   savedNumber = atoi(tokenString);
                   savedLineNo = lineno;
                 }
            ;
vardecl    : typespec forid SEMI
                 { 
				   $$ = newDeclNode(VarK);
                   $$->child[0] = $1;
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                 }
            | typespec forid LBRACE fornum RBRACE SEMI
                 { 
				   $$ = newDeclNode(ArrVarK);
                   $$->child[0] = $1;
                   $$->lineno = lineno;
                   $$->attr.arr.name = savedName;
                   $$->attr.arr.size = savedNumber;
                 }
            ;
typespec   : INT
                 { 
			       $$ = newTypeNode(TypingK);
                   $$->attr.type = INT;
                 }
            | VOID
                 { 
                   $$ = newTypeNode(TypingK);
                   $$->attr.type = VOID;
                 }
            ;
fundecl    : typespec forid 
				 {
                   $$ = newDeclNode(FuncK);
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                 }
              LPAREN params RPAREN compstmt
                 {
                   $$ = $3;
                   $$->child[0] = $1; 
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
            ;
params      : paramlist  { $$ = $1; }
            | VOID
                 { 
				   $$ = newTypeNode(TypingK);
                   $$->attr.type = VOID;
                 }
paramlist  : paramlist COMMA param
                 { 
				   YYSTYPE t = $1;
                   if (t != NULL)
                   { 
					 while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; 
				   }
                   else 
					 $$ = $3; 
                 }
            | param { $$ = $1; };
param       : typespec forid
                 { 
				   $$ = newDeclNode(ParK);
                   $$->child[0] = $1;
                   $$->attr.name = savedName;
                 }
            | typespec forid
              LBRACE RBRACE
                 { 
			       $$ = newDeclNode(ArrParK);
                   $$->child[0] = $1;
                   $$->attr.name = savedName;
                 }
            ;
compstmt   : LCURLY localdecls stmtlist RCURLY
                 { 
			       $$ = newStmtNode(CompK);
                   $$->child[0] = $2; 
                   $$->child[1] = $3; 
                 }
            ;
localdecls : localdecls vardecl
              	 { 
				   YYSTYPE t = $1;
                   if (t != NULL)
                   {
				     while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; 
				   }
                   else 
					 $$ = $2;
                 }
            | { $$ = NULL; }
            ;
stmtlist   : stmtlist stmt
                 { 
				   YYSTYPE t = $1;
                   if (t != NULL)
                   { 
					 while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; 
				   }
                   else 
					 $$ = $2;
                 }
            | { $$ = NULL; }
            ;
stmt        : expstmt { $$ = $1; }
            | compstmt { $$ = $1; }
            | selstmt { $$ = $1; }
            | iterstmt { $$ = $1; }
            | retstmt { $$ = $1; }
            ;
expstmt    : exp SEMI { $$ = $1; }
            | SEMI { $$ = NULL; }
            ;
selstmt    : IF LPAREN exp RPAREN stmt
                 { 
				   $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = NULL;
                 }
            | IF LPAREN exp RPAREN stmt ELSE stmt
                 { 
				   $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
            ;
iterstmt   : WHILE LPAREN exp RPAREN stmt
                 { 
			       $$ = newStmtNode(IterK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                 }
            ;
retstmt    : RETURN SEMI
                 {
				   $$ = newStmtNode(RetK);
                   $$->child[0] = NULL;
                 }
            | RETURN exp SEMI
                 {
				   $$ = newStmtNode(RetK);
                   $$->child[0] = $2;
                 }
            ;
exp         : var ASSIGN exp
                 {
				   $$ = newExpNode(AssignK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
            | simpleexp { $$ = $1; }
            ;
var         : forid 
                 {
				   $$ = newExpNode(IdK);
                   $$->attr.name = savedName;
                 }
            | forid 
                 {
				   $$ = newExpNode(ArrIdK);
                   $$->attr.name = savedName;
                 }
              LBRACE exp RBRACE
                 { 
					$$ = $2;
					$$->child[0] = $4;
                 }
            ;
simpleexp  : addexp LE addexp
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LE;
                 }
            | addexp LT addexp
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LT;
                 }
            | addexp GT addexp
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = GT;
                 }
            | addexp GE addexp
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = GE;
                 }
            | addexp EQ addexp
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = EQ;
                 }
            | addexp NE addexp
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = NE;
                 }
            | addexp { $$ = $1; }
            ;
addexp     : addexp PLUS term
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = PLUS;
                 }
            | addexp MINUS term
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = MINUS;
                 }
            | term { $$ = $1; }
            ;
term        : term TIMES factor
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = TIMES;
                 }
            | term OVER factor
                 {
				   $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = OVER;
                 }
            | factor { $$ = $1; }
            ;
factor      : LPAREN exp RPAREN { $$ = $2; }
            | var { $$ = $1; }
            | call { $$ = $1; }
            | NUM
                 {
				   $$ = newExpNode(ConstK);
					savedName = copyString(tokenString);
                   $$->attr.val = atoi(savedName);
                 }
            ;
call        : forid 
			  {
                 $$ = newExpNode(CallK);
                 $$->attr.name = savedName;
              }
              LPAREN args RPAREN
                 {
				   $$ = $2;
                   $$->child[0] = $4;
                 }
            ;
args        : arg_list { $$ = $1; }
            | { $$ = NULL; }
            ;
arg_list    : arg_list COMMA exp
                 { 
				   YYSTYPE t = $1;
                   if (t != NULL)
                   { 
					 while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1;
				   }
                   else 
					 $$ = $3;
                 }
            | exp { $$ = $1; }
            ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

