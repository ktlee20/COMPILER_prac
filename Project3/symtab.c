#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"

Scope ScopeList[SIZE];
Stack * stack;

int hash(char * key)
{
	int temp = 0;
	int i =0;

	while(key[i] != '\0')
	{
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		i++;
	}
	return temp;
}

void SInit(Stack ** s)
{
	*s = (Stack*)malloc(sizeof(Stack));
	(*s)->head = NULL;
	(*s)->numOfSData = 0;
}

void SPush(Stack * s, SData data)
{
	SNode * temp = (SNode *)malloc(sizeof(SNode));
	char * tmp = (char*)malloc(sizeof(char) * strlen(data));;
	strcpy(tmp, data);	
	
	if(s->head == NULL)
	{
		s->head = temp;
		s->numOfSData++;
		s->head->data = tmp;
		s->head->prev = NULL;

		return;
	}
	temp->data = tmp;
	temp->prev = stack->head;
	stack->head = temp;
	stack->numOfSData++;
}

SData SPop(Stack * s)
{
	SNode * temp;
	SData tmp;

	if(s->head == NULL)
		return 0;
	
	temp = stack->head;
	tmp = stack->head->data;
	s->numOfSData--;
	s->head = temp->prev;
	free(temp);

	return tmp;
}

SData SPeek(Stack * s)
{
	return (s->head == NULL) ? NULL : s->head->data;
}

int SIsEmpty(Stack * s)
{
	if(s->numOfSData == 0)
		return 1;

	return 0;
}

int numOfStack(Stack * s)
{
	return s->numOfSData;
}

int process_locate(char * scope)
{
	int h = hash(scope);
	Scope s = ScopeList[h];
	
	if(s == NULL)
		return 0;

	return s->location++;
}

Scope sc_find(char * name)
{
	char * scope = SPeek(stack);
	int h1 = hash(name), h2 = hash(scope);	
	Scope s = ScopeList[h2];

	while(s != NULL)
	{
		BucketList l = s->bucket[h1];
		while((l != NULL) && (strcmp(name, l->name) != 0))
			l = l->next;
		if(l != NULL)
			return s;

		s = s->parent;
	}
	return NULL;	
}

BucketList bc_lookup(char * scope, char * name)
{
	int h1 = hash(scope), h2 = hash(name);
	Scope s = ScopeList[h1];

	while(s != NULL)
	{
		BucketList l = s->bucket[h2];	
		while((l != NULL) && (strcmp(name, l->name) != 0))
			l = l->next;
		if(l != NULL)
			return l;

		s = s->parent;
	}
	return NULL;
}	

BucketList bc_lookup_excluding_parent(char * scope, char * name)
{
	int h1 = hash(scope), h2 = hash(name);	
	Scope s = ScopeList[h1];
	BucketList l;	

	if(s == NULL)
		return NULL;

	l = s->bucket[h2];
	while((l != NULL) && (strcmp(name, l->name) != 0))
		l = l->next;

	return l;
}

Scope sc_insert(char * name, TreeNode * t)
{
	int h = hash(name), i;
	char * pname = SPeek(stack);
	Scope s = ScopeList[h];

	while(s != NULL && (strcmp(name, s->name) != 0))
		s = s->next;
	
	/*if(s != NULL)
		return NULL; //error
	*/

	s = (Scope)malloc(sizeof(struct ScopeListRec));
	s->name = name;
	s->t = t;
	s->next = ScopeList[h];
	s->location = 0;
	s->parent = (pname == NULL) ? NULL : ScopeList[hash(pname)];
	for(i = 0 ; i < SIZE ; i++)
		s->bucket[i] = NULL;
	

	ScopeList[h] = s;

	return s;
}

LineList bc_varline(char * scope, char * name, int lineno, TreeNode * t)
{
	int h1 = hash(scope), h2 = hash(name);
	Scope s = ScopeList[h1];	
	BucketList l;
	LineList ll;

	while(s != NULL)
	{
		l = s->bucket[h2];	
		while((l != NULL) && (strcmp(name, l->name) != 0))
			l = l->next;
		if(l != NULL)
			break;
		
		s = s->parent;
	}
	
	if(s == NULL)
		return NULL;

	if(l == NULL)
		return NULL;

	ll = l->lines;
	if(ll == NULL)
		return NULL;

	while(ll->next != NULL)
		ll = ll->next;
	
	ll->next = (LineList) malloc(sizeof(struct LineListRec));	
	ll->next->lineno = lineno;
	ll->next->next = NULL;

	return ll;
}

BucketList bc_insert(char * scope, char * name, ExpType type, int lineno, int loc, TreeNode * t)
{
	int h1 = hash(scope), h2 = hash(name);
	Scope s = ScopeList[h1];
	BucketList l;
	LineList ll;

	while(s != NULL && (strcmp(scope, s->name) != 0))
		s = s->next;
	
	if(s == NULL)
		return NULL;

	l = s->bucket[h2];

	while(l != NULL && (strcmp(name , l->name) != 0))
		l = l->next;
	
	if(l == NULL)
	{
		l = (BucketList)malloc(sizeof(struct BucketListRec));
		l->name = name;
		l->lines = (LineList)malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->memloc = loc;
		l->lines->next = NULL;
		l->t = t;
		l->next = s->bucket[h2];
		l->type = type;
		s->bucket[h2] = l;
	}

	return l;
}

void printSymTab(FILE * listing)
{
	int i,j;
	char a1[10] = "Integer";
	char a2[10] = "Void";
	char a3[20] = "IntegerArray";
	for(i = 0 ; i < SIZE ; i++)
	{
		if(ScopeList[i] != NULL)
		{
			Scope s = ScopeList[i];
			fprintf(listing, "Scope name : "); 
			fprintf(listing, "~");
			if(strcmp(s->name, "~") != 0)
				fprintf(listing, ":%s\n",s->name);
			else
				fprintf(listing, "\n");
			fprintf(listing, "Variable Name  Variable Type  Location       Line Numbers    \n"); 
			fprintf(listing, "-------------- -------------- -------------- --------------- \n");
			for(j = 0 ; j < SIZE ; j++)
			{
				BucketList l = s->bucket[j];
				if(l != NULL)
				{
					LineList ll = l->lines;
					fprintf(listing,"%-14s ",l->name);
					if(l->type == Int)
						fprintf(listing, "%-15s",a1);
					else if(l->type == IntArray)
						fprintf(listing, "%-15s",a3);
					else
						fprintf(listing, "%-15s",a2);
						
					fprintf(listing,"%-14d ",l->memloc);
					while(ll != NULL)
					{
						fprintf(listing,"%-4d ",ll->lineno);
						ll = ll->next;
					}
					fprintf(listing, "\n");
					l = l->next;
				}
			}	
			fprintf(listing,"--------------------------------------------------------------\n");
			fprintf(listing, "\n");
		}
	}
}
