/****************************************************/
/* File: util.h                                     */
/* Utility functions for the TINY compiler          */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

void printToken( TokenType, const char* );

TreeNode * newStmtNode(StmtKind);

TreeNode * newExpNode(ExpKind);

TreeNode * newDeclNode(DeclKind);

TreeNode * newTypeNode(TypeKind);

char * copyString( char * );

void printTree( TreeNode * );

#endif
