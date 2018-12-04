/****************************************************/
/* File: analyze.h                                  */
/* Semantic analyzer interface for TINY compiler    */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "globals.h"
#include "symtab.h"
#include "util.h"

extern char * funname;
extern int scopemade;

static void traverse(TreeNode * t, void(* preProc) (TreeNode *), void (*postProc) (TreeNode*));
static void reserveIO(void);
static void nullProc(TreeNode * t);
static void symbolError(TreeNode * t, char * message);
static void insertNode( TreeNode * t);
static void pop_scope(TreeNode * t);
void buildSymtab(TreeNode *);
static void push_scope(TreeNode * t);
static void typeError(TreeNode *, char * message);
static void checkNode(TreeNode * t);
void typeCheck(TreeNode *);

#endif
