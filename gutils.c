/* -- gutils.c -- a stack based programming language */
#include <stdarg.h>
#include <stdlib.h>
#include "gstack.h"

void gfatal_error(const char* format, ...)
{
	fprintf(stderr, "FATAL ERROR:\n");
	va_list args;
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
	exit(1);
}

void gdebug_expr(gexpr_t* exp)
{
	gexpr_t* first;
	switch(exp->type)
	{
	case EXPR_NUMBER:
		printf("number expression:\n");
		printf("	value: %f\n", exp->numexpr.value); 
		break;
	case EXPR_BOOLEAN:
		printf("boolean expression:\n");
		printf("	value: %s\n", exp->boolexpr.value ? "true" : "false");
		break;
	case EXPR_STRING:
		printf("string expression:\n");
		printf("	value: %s\n", exp->strexpr.value);
		break;
	case EXPR_SYMBOL:
		printf("symbol expression:\n");
		printf("	value: %s\n", exp->symexpr.value);
		break;
	case EXPR_CALL:
		printf("call expression\n");
		printf("	value: %s\n", exp->callexpr.value);
		break;
	case EXPR_DEFINE:
		printf("define expression\n");
		printf("	name: %s\n", exp->defexpr.name);
		printf("	block:\n");
		gdebug_expr(exp->defexpr.block);
		break;
	case EXPR_WHILE:
		printf("while expression\n");
		printf("	condition:\n");
		gdebug_expr(exp->whileexpr.cond);
		printf("	block:\n");
		gdebug_expr(exp->whileexpr.block);
	case EXPR_IF:
		printf("if expression\n");
		printf("	condition:\n");
		gdebug_expr(exp->ifexpr.cond);
		printf("	true block:\n");
		gdebug_expr(exp->ifexpr.true_block);
		if(exp->ifexpr.false_block)
		{
			printf("	false block:\n");
			gdebug_expr(exp->ifexpr.false_block);
		}
		break;
	case EXPR_BLOCK:
		printf("block expression:\n");
		first = exp->blockexpr.block_head;
		while(first != NULL)
		{
			gdebug_expr(first);
			first = first->next;
		}
		break;
	default:
		break;
	}
}
