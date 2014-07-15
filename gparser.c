/* -- gparser.c -- a stack based programming language */
#include <stdlib.h>
#include <string.h>
#include "gstack.h"

static void gexpect(gtoken_type_t type)
{
	if(gtoken.type != type)
		gfatal_error("expected token %i, but recieved %i\n", type, gtoken.type);
}

gexpr_t* gmake_expr(gexpr_type_t type)
{
	gexpr_t* exp = malloc(sizeof(gexpr_t));
	if(!exp)
		gfatal_error("out of memory\n");
	exp->type = type;
	exp->next = NULL;
	return exp;
}

gexpr_t* gexpr_number()
{
	gexpr_t* exp = gmake_expr(EXPR_NUMBER);
	exp->numexpr.value = gtoken.number;
	gread_token();
	return exp;
}

gexpr_t* gexpr_boolean()
{
	gexpr_t* exp = gmake_expr(EXPR_BOOLEAN);
	exp->boolexpr.value = gtoken.boolean;
	gread_token();
	return exp;
}

gexpr_t* gexpr_string()
{
	gexpr_t* exp = gmake_expr(EXPR_STRING);
	exp->strexpr.value = strdup(gtoken.buffer);
	exp->strexpr.length = gtoken.length;
	gread_token();
	return exp;
}

gexpr_t* gexpr_symbol()
{
	gexpr_t* exp = gmake_expr(EXPR_SYMBOL);
	exp->symexpr.value = strdup(gtoken.buffer);
	gread_token();
	return exp;
}

gexpr_t* gexpr_call()
{
	gexpr_t* exp = gmake_expr(EXPR_CALL);
	exp->callexpr.value = strdup(gtoken.buffer);
	gread_token();
	return exp;
}

gexpr_t* gexpr_define()
{
	gread_token();
	gexpect(TOK_SYMBOL);
	char* name = strdup(gtoken.buffer);
	gread_token();
	gexpr_t* block = gexpr_block();
	gexpr_t* exp = gmake_expr(EXPR_DEFINE);
	exp->defexpr.name = name;
	exp->defexpr.block = block;
	return exp;
}

gexpr_t* gexpr_while()
{
	gread_token();
	gexpr_t* cond = gexpr_block();
	gexpr_t* block = gexpr_block();
	gexpr_t* exp = gmake_expr(EXPR_WHILE);
	exp->whileexpr.cond = cond;
	exp->whileexpr.block = block;
	return exp;
}

gexpr_t* gexpr_if()
{
	gread_token();
	gexpr_t* cond = gexpr_block();
	gexpr_t* true_block = gexpression();
	gexpr_t* false_block = NULL;
	if(gtoken.type == TOK_ELSE)
	{
		gread_token();
		false_block = gexpression();
	}
	gexpr_t* exp = gmake_expr(EXPR_IF);
	exp->ifexpr.cond = cond;
	exp->ifexpr.true_expr = true_block;
	exp->ifexpr.false_expr = false_block;
	return exp;
}

gexpr_t* gexpr_block()
{
	gexpect(TOK_OPENBLOCK);
	gread_token();
	gexpr_t* exp = gmake_expr(EXPR_BLOCK);
	if(gtoken.type == TOK_CLOSEBLOCK) 
	{
		exp->blockexpr.block_head = NULL;
		gread_token();
		return exp;
	}
	gexpr_t* expr_head = gexpression();
	gexpr_t* last = expr_head;
	while(gtoken.type != TOK_CLOSEBLOCK)
	{
		gexpr_t* bexp = gexpression();
		last->next = bexp;
		last = bexp;
	}
	last->next = NULL;
	gread_token();
	exp->blockexpr.block_head = expr_head;
	return exp;
}

gexpr_t* gexpr_set()
{
	gread_token();
	gexpect(TOK_SYMBOL);
	char* name = strdup(gtoken.buffer);
	gexpr_t* exp = gmake_expr(EXPR_SET);
	exp->setexpr.name = name;
	gread_token();
	return exp;
}

gexpr_t* gexpr_create()
{
	gread_token();
	char* name = strdup(gtoken.buffer);
	gexpr_t* exp = gmake_expr(EXPR_CREATE);
	exp->createexpr.name = name;
	gread_token();
	return exp; 
}

gexpr_t* gexpr_pair()
{
	gexpect(TOK_OPENPAIR);
	gread_token();
	gexpr_t* exp = gmake_expr(EXPR_PAIR);
	if(gtoken.type == TOK_CLOSEPAIR)
	{
		exp->pairexpr.head = NULL;
		exp->pairexpr.tail = NULL;
		return exp;
	}
	exp->pairexpr.head = gexpression();
	exp->pairexpr.tail = gexpression();
	gexpect(TOK_CLOSEPAIR);
	gread_token();
	return exp;
}

gexpr_t* gexpression()
{
	switch(gtoken.type)
	{
	case TOK_NUMBER:
		return gexpr_number();
	case TOK_BOOLEAN:
		return gexpr_boolean();
	case TOK_STRING:
		return gexpr_string();
	case TOK_SYMBOL:
		return gexpr_symbol();
	case TOK_CALL:
		return gexpr_call();
	case TOK_DEFINE:
		return gexpr_define();
	case TOK_WHILE:
		return gexpr_while();
	case TOK_IF:
		return gexpr_if();
	case TOK_OPENBLOCK:
		return gexpr_block();
	case TOK_SET:
		return gexpr_set();
	case TOK_CREATE:
		return gexpr_create();
	case TOK_OPENPAIR:
		return gexpr_pair();
	case TOK_EOF:
		return NULL;
	default:
		gfatal_error("invalid top level token (type %i)\n", gtoken.type);
	}
	gfatal_error("gexpression invalid state\n");
	return NULL;
}

void gdestroy_expr(gexpr_t* exp)
{
	if(!exp) return;
	gexpr_t* first;
	switch(exp->type)
	{
	case EXPR_STRING:
		free(exp->strexpr.value);
		exp->strexpr.value = NULL;
		break;
	case EXPR_SYMBOL:
		free(exp->symexpr.value);
		exp->symexpr.value = NULL;
		break;
	case EXPR_CALL:
		free(exp->callexpr.value);
		exp->callexpr.value = NULL;
		break;
	case EXPR_DEFINE:
		free(exp->defexpr.name);
		exp->defexpr.name = NULL;
		gdestroy_expr(exp->defexpr.block);
		break;
	case EXPR_WHILE:
		gdestroy_expr(exp->whileexpr.cond);
		gdestroy_expr(exp->whileexpr.block);
		break;
	case EXPR_IF:
		gdestroy_expr(exp->ifexpr.cond);
		gdestroy_expr(exp->ifexpr.true_expr);
		if(exp->ifexpr.false_expr)
			gdestroy_expr(exp->ifexpr.false_expr);
		break;
	case EXPR_BLOCK:
		first = exp->blockexpr.block_head;
		while(first != NULL)
		{
			gexpr_t* next = first->next;
			gdestroy_expr(first);
			first = next;
		}
		break;
	case EXPR_SET:
		free(exp->setexpr.name);
		exp->setexpr.name = NULL;
		break;
	case EXPR_CREATE:
		free(exp->createexpr.name);
		exp->createexpr.name = NULL;
		break;
	case EXPR_PAIR:
		gdestroy_expr(exp->pairexpr.head);
		gdestroy_expr(exp->pairexpr.tail);
		break;
	default:
		break;
	}
	
	if(exp)
		free(exp);
}
