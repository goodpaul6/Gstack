/* -- gexecute.c -- a stack based programming language */
#include <stdlib.h>
#include <string.h>
#include "gstack.h"

static char break_triggered = 0;
static char continue_triggered = 0;

static char is_true(gobject_t* obj)
{
	if(obj->type == OBJ_NUMBER)
		return (obj->number.value != 0);
	if(obj->type == OBJ_BOOLEAN)
		return obj->boolean.value;
	return 1;
}

void gnode_do(gstate_t* state, gexpr_t* exp)
{
	gsymbol_t* sym;
	gexpr_t* node;
	switch(exp->type)
	{
	case EXPR_NUMBER:
		gpush_number(state, exp->numexpr.value);
		break;
	case EXPR_BOOLEAN:
		gpush_boolean(state, exp->boolexpr.value);
		break;
	case EXPR_STRING:
		gpush_string(state, exp->strexpr.value, exp->strexpr.length, 1);
		break;
	case EXPR_SYMBOL:
		if(strcmp(exp->symexpr.value, "break") == 0)
		{
			break_triggered = 1;
			return;
		}
		
		if(strcmp(exp->symexpr.value, "continue") == 0)
		{
			continue_triggered = 1;
			return;
		}
		
		sym = gsymbol_get(state, exp->symexpr.value);
		if(sym->value)
			gpush_object(state, sym->value);
		else
			gfatal_error("attempted to push a symbol (%s) with no value\n", exp->symexpr.value);
		break;
	case EXPR_CALL:
		gcall_function(state, exp->callexpr.value);
		break;
	case EXPR_DEFINE:
		break;
	case EXPR_WHILE:
		gnode_do(state, exp->whileexpr.cond);
		while(is_true(gpop_object(state)))
		{
			gnode_do(state, exp->whileexpr.block);
			if(break_triggered) 
			{ 
				break_triggered = 0;
				break;
			}
			gnode_do(state, exp->whileexpr.cond);
		}
		break;
	case EXPR_IF:
		gnode_do(state, exp->ifexpr.cond);
		if(is_true(gpop_object(state)))
			gnode_do(state, exp->ifexpr.true_expr);
		else if(exp->ifexpr.false_expr)
			gnode_do(state, exp->ifexpr.false_expr);
		break;
	case EXPR_BLOCK:
		gsymbol_push(state);
		node = exp->blockexpr.block_head;
		while(node != NULL)
		{	
			gnode_do(state, node);
			node = node->next;
		}
		gsymbol_pop(state);
		break;
	case EXPR_SET:
		sym = gsymbol_get(state, exp->setexpr.name);
		sym->value = gpop_object(state);
		break;
	case EXPR_CREATE:
		sym = gsymbol_create(state, exp->createexpr.name);
		sym->value = gpop_object(state);
		break;
	case EXPR_PAIR:
		if(exp->pairexpr.head)
			gnode_do(state, exp->pairexpr.head);
		if(exp->pairexpr.tail)
			gnode_do(state, exp->pairexpr.tail);
		gpush_pair(state);
		break;
	}
}

void gcall_function(gstate_t* state, const char* name)
{
	gexpr_t* defexpr = gdefine_get(state, name);
	if(defexpr)
	{
		gnode_do(state, defexpr->defexpr.block);
		return;
	}
	gprimitive_t* prim = gprimitive_get(state, name);
	if(prim)
	{
		prim->fn(state);
		return;
	}
	gfatal_error("attempted to call non-existent function/define (%s)\n", name);
}

void gexecute_program(gstate_t* state)
{
	gexpr_t* prog_node = state->program_head;
	while(prog_node)
	{
		gnode_do(state, prog_node);
		prog_node = prog_node->next;
	}
}
