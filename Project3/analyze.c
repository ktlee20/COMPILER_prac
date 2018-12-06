#include <string.h>
#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

char * funname;
int scopemade = 0;
int funexist = 0;

static void traverse(TreeNode * t, void(*preProc) (TreeNode*), void (*postProc) (TreeNode*))
{
	if(t != NULL)
	{
		preProc(t);
		{
			int i;
			for(i = 0 ; i < MAXCHILDREN ; i++)
			{
				funexist = 0;
				traverse(t->child[i],preProc, postProc);
				if(funexist)
					break;
			}
		}
		postProc(t);
		traverse(t->sibling, preProc, postProc);
	}
}

static void reserveIO(void)
{
	TreeNode * temp;
	temp = newDeclNode(FuncK);
	
	temp->lineno = 0;
	temp->type = Int;
	temp->attr.name = "input";

	temp->child[0] = newTypeNode(FuncK);
	temp->child[0]->attr.type = INT;

	temp->child[1] = NULL;

	temp->child[2] = newStmtNode(CompK);
	temp->child[2]->child[0] = NULL;
	temp->child[2]->child[1] = NULL;
	
	bc_insert("~","input",Int, 0, process_locate("~"), temp);
	sc_insert("input", temp);	

	temp = newDeclNode(FuncK);

	temp->lineno = 0;
	temp->type = Void;		
	temp->attr.name = "output";

	temp->child[0] = newTypeNode(FuncK);
	temp->child[0]->attr.type = Void;

	temp->child[1] = newDeclNode(ParK);
	temp->child[1]->attr.name = "arg";
	temp->child[1]->child[0] = newTypeNode(FuncK);
	temp->child[1]->child[0]->attr.type = INT;

	temp->child[2] = newStmtNode(CompK);
	temp->child[2]->child[0] = NULL;
	temp->child[2]->child[1] = NULL;

	bc_insert("~","output",Void, 0, process_locate("~"), temp);
	sc_insert("output", temp);
	bc_insert("output","arg",Int,0,process_locate("output"), temp);
}

static void nullProc(TreeNode * t)
{
	if (t ==NULL)
		return;
	else
		return;
}

static void insertNode( TreeNode * t)
{
	char *name;
	switch(t->nodekind)
	{
		case StmtK:
		{
			switch (t->kind.stmt)
			{
				case CompK:
					if(scopemade || funexist)
					{
						scopemade = 0;
						funexist = 0;
					}
					else
					{
						Scope s;
						BucketList l;
						int h = hash(funname);
						s = ScopeList[h]->parent;
						l = s->bucket[h];	
						name = (char*)malloc(sizeof(char) * 256);
						strcpy(name, funname);
						sprintf(name,"%s:%d",name,t->lineno);
						funname = name;
						bc_insert(SPeek(stack), funname, l->type, t->lineno, process_locate(SPeek(stack)), t);
						s = sc_insert(name, t);
						SPush(stack, funname);
					}
					t->attr.scope = SPeek(stack);
					break;
				default:
					break;
			}
		}
		break;
		case ExpK:
		{
			switch(t->kind.exp)
			{
				case IdK:
				case ArrIdK:
				case CallK:
					if(bc_lookup(SPeek(stack), t->attr.name) == NULL)
						symbolError(t, "symbol doesn't exist");
					else
						bc_varline(SPeek(stack),t->attr.name, t->lineno, t); 
				break;
				default:
					break;
			}
		}
		break;
		case DclK:
		{
			switch(t->kind.decl)
			{
				case FuncK:
					funname = t->attr.name;
					if(bc_lookup(SPeek(stack), funname) != NULL)
					{
						symbolError(t, "function exists");
						funexist = 1;			
						break;
					}
					switch(t->child[0]->attr.type)
					{
						case INT:
							t->type = Int;
							break;
						case VOID:
						default:
							t->type = Void;
							break;
					}
					bc_insert(SPeek(stack), funname, t->type, t->lineno, process_locate(SPeek(stack)), t);
					scopemade = 1;	
					sc_insert(funname, t);
					SPush(stack, funname);
				break;
				case VarK:
				{
					if(t->child[0]->attr.type == VOID)
					{
						symbolError(t, " variable should have non-void type");
						break;
					}
					t->type = Int;

					if(bc_lookup_excluding_parent(SPeek(stack),t->attr.name) == NULL)
						bc_insert(SPeek(stack), t->attr.name,Int,t->lineno,process_locate(SPeek(stack)), t);
					else
						symbolError(t, "symbol already exists");
				}
				break;
				case ArrVarK:
				{
					if(t->child[0]->attr.type == VOID)
					{
						symbolError(t, " variable should have non-void type");
						break;
					}
					t->type = IntArray;
					if(bc_lookup_excluding_parent(SPeek(stack), t->attr.name) == NULL)
						bc_insert(SPeek(stack), t->attr.name, IntArray, t->lineno, process_locate(SPeek(stack)), t);
					else
						symbolError(t, "symbol already exists");
				}
				break;
				case ParK:
				{
					if(t->child[0]->attr.type == VOID)
						symbolError(t->child[0], "void parameter");
					t->type = Int;
					if(bc_lookup_excluding_parent(SPeek(stack), t->attr.name) == NULL)
						bc_insert(SPeek(stack), t->attr.name, Int, t->lineno, process_locate(SPeek(stack)), t);
					else
						symbolError(t, "symbol already exists");
						
				}
				break;
				case ArrParK:
				{
					if(t->child[0]->attr.type == VOID)
						symbolError(t->child[0], "void parameter");
					t->type = IntArray;
					if(bc_lookup_excluding_parent(SPeek(stack), t->attr.name) == NULL)
						bc_insert(SPeek(stack), t->attr.name, IntArray, t->lineno, process_locate(SPeek(stack)), t);
					else
						symbolError(t, "symbol already exists");
				}
				break;
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
}

static void symbolError(TreeNode *t, char * message)
{
	fprintf(listing, "Symbol error at line %d: %s\n",t->lineno, message);
	Error = TRUE;
}

static void pop_scope(TreeNode * t)
{
	switch(t->nodekind)
	{
		case StmtK:
		{
			switch(t->kind.stmt)
			{
				case CompK:
					if(funexist)
						funexist = 0;
					else
						SPop(stack);
					funname = SPeek(stack);
				break;
				default:
				break;
			}
		}	
		break;
		default:
		break;
	}
}

void buildSymtab(TreeNode * syntaxTree)
{
	char cmd[12] = "~";

	SInit(&stack);
	sc_insert(cmd, NULL);
	SPush(stack, cmd);
	scopemade = 1;
	reserveIO();
	traverse(syntaxTree,insertNode ,pop_scope);
	SPop(stack);
	if(TraceAnalyze)
	{
		fprintf(listing, "\nSymbol table:\n\n");
		printSymTab(listing);
	}
}

static void push_scope(TreeNode *t)
{
	switch(t->nodekind)
	{
		case StmtK:
			switch(t->kind.stmt)
			{
				case CompK:
					if(t->attr.scope != NULL)
						SPush(stack, t->attr.scope);
				break;
				default:
				break;
			}
		break;
		case DclK:
			switch(t->kind.decl)
			{
				case FuncK:
					funname = t->attr.name;
				break;
				default:
				break;
			}
		break;
		default:
		break;
	}
}

static void typeError(TreeNode * t, char * message)
{
	fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
	Error = TRUE;
}

static void checkNode(TreeNode *t)
{
	int i;
	switch(t->nodekind)
	{
		case StmtK:
			switch (t->kind.stmt)
			{
				case CompK:
					if(stack->head->data[0] != '~');
						SPop(stack);	
				break;
				case RetK:
				{
					BucketList l  = bc_lookup(funname,funname);

					if(l->type == Void && (t->child[0] != NULL && t->child[0]->type != Void))
						typeError(t, "no return value");
					else if(l->type == Int && (t->child[0] == NULL || t->child[0]->type == Void))
						typeError(t, "return value");
				}
				break;
				default:
				break;
			}
			break;
		case ExpK:
			switch(t->kind.exp)
			{
				case AssignK:
				{
					TreeNode * left = t->child[0];
					TreeNode * right = t->child[1];
					BucketList l = bc_lookup(SPeek(stack), left->attr.name);

					if(l == NULL)	
						break;				

					if(l->type == IntArray && left->child[0] == NULL)
					{
						typeError(t, "cannot assign");
						break;
					}
					if(right->type == Void)
					{
						typeError(t, "cannot assign");
						break;
					}
					t->type = t->child[0]->type;
				}
				break;
				case OpK:
				{
					TreeNode * left = t->child[0];
					TreeNode * right = t->child[1];
					BucketList l;					
					BucketList r;
					
					if(left->kind.exp == ConstK || left->kind.exp == OpK)
						l = NULL;
					else
						l = bc_lookup(SPeek(stack), left->attr.name);


					if(right->kind.exp == ConstK || right->kind.exp == OpK)
						r = NULL;
					else
						r = bc_lookup(SPeek(stack), right->attr.name);	
	
					if(right->type == Int && left->type == Int)
					{
						t->type = Int;
						break;
					}
					
					if(l == NULL || r == NULL)
						break;					

					if(l->type == IntArray && left->child[0] == NULL)
					{
						typeError(t, "cannot op");
						break;
					}
					if(r->type == IntArray && right->child[0] == NULL)
					{
						typeError(t, "cannot op");
						break;
					}

					if(t->child[0]->type == Void || t->child[1]->type == Void)
						typeError(t, "operands should be non-void");
					else
						t->type = Int;
				}
				break;
				case ConstK:
					t->type = Int;
				break;
				case IdK:
				{
					BucketList l = bc_lookup(SPeek(stack), t->attr.name);
					
					if(l == NULL)
						break;
						
						t->type = Int;
						break;
				}
				break;
				case ArrIdK:
				{
					BucketList l = bc_lookup(SPeek(stack), t->attr.name);

					if(l == NULL)
						break;
					
					if(t->child[0] == NULL)
						typeError(t, "array should be used with index");
					if(t->child[0]->type != Int)
						typeError(t, "index should be integer");			
					else
					{
						t->type = IntArray;
						l->type = Int;
					}
				}
				break;
				case CallK:
				{
					BucketList l = bc_lookup(SPeek(stack), t->attr.name);
					TreeNode * par1,*par2;		
	
					if(l == NULL)
						break;
					
					if(l->t == NULL)
						break;	
	
					par1 = t->child[0];			
					par2 = l->t->child[1];

					if(l->t->kind.decl != FuncK)
					{
						typeError(t, "need function");
						break;
					}
					while(par1 != NULL)
					{
						/*if(par1->type == Void)
						{
							typeError(par1, "void cannot be parameter");
							break;
						}*/
						if(par2 == NULL)
						{
							typeError(par1, "parameter number doesn't match");
							break;
						}
						par1 = par1->sibling;
						par2 = par2->sibling;
					}
			
					if(par1 == NULL && (par2 != NULL && par2->type != Void))
						typeError(t, "parameter number doesn't match");

					t->type = l->t->type;
				}
				break;
				default:
				break;
			}
		break;
		default:
		break;
	}
}

void typeCheck(TreeNode * syntaxTree)
{
	char cmd[12] = "~";

	SInit(&stack);
	SPush(stack, cmd);
	traverse(syntaxTree,push_scope,checkNode);
	SPop(stack);
}
