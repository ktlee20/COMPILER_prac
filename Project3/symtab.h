/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"
#define SIZE 1000 
#define SHIFT 4
typedef char* SData;

typedef struct _snode
{
	 SData data;
	 struct _snode *prev;
}SNode;

typedef struct _stack
{
	SNode * head;
	int numOfSData;
} Stack;

typedef struct LineListRec
{
	int lineno;
	struct LineListRec * next;
} *LineList;

typedef struct BucketListRec
{
	char * name;
	ExpType type;
	LineList lines;
	int memloc;
	struct BucketListRec * next;
	struct treeNode * t;
} * BucketList;

typedef struct ScopeListRec
{
	int location;
	char * name;
	BucketList bucket[SIZE];
	struct ScopeListRec * next;
	struct ScopeListRec * parent;
	struct treeNode * t;
} *Scope;

extern Scope ScopeList[SIZE];
extern Stack *stack;

int hash( char * key);
void SInit(Stack ** s);
void SPush(Stack * s, SData data);
SData SPop(Stack * s);
SData SPeek(Stack * s);
int SIsEmpty(Stack * s);
int numOfStack(Stack * s);
int process_locate(char * scope);
Scope sc_find(char * funcName);
Scope sc_insert(char * funcName, TreeNode * t);
LineList bc_varline(char * scope, char * name, int lineno,TreeNode * t);
BucketList bc_lookup(char * scope, char * name);
BucketList bc_lookup_excluding_parent(char * scope, char * name);
BucketList bc_insert( char * scope,  char * name, ExpType type ,int lineno, int loc, TreeNode * t );
void printSymTab(FILE * listing);

#endif
