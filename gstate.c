/* -- gstate.c -- a stack based programming language */
#include <stdlib.h>
#include <string.h>
#include "gstack.h"

gstate_t* gmake_state()
{
	gstate_t* state = malloc(sizeof(gstate_t));
	if(!state) gfatal_error("out of memory\n");
	state->object_amt = 0;
	state->max_object_amt = INITIAL_GC_THRES;
	state->primitives_head = NULL;
	state->gc_head = NULL;
	state->program_head = NULL;
	state->stack_size = 0;
	state->symbols_stack[0] = NULL;
	state->symbols_stack_size = 0;
	return state;
}

void gdestroy_state(gstate_t* state)
{
	state->stack_size = 0;
	gsymbol_t* sym = state->symbols_stack[0];
	while(sym)
	{
		gsymbol_t* next = sym->next;
		free(sym);
		sym = next;
	}
	state->symbols_stack[0] = NULL;
	
	gprimitive_t* prim = state->primitives_head;
	while(prim)
	{
		gprimitive_t* next = prim->next;
		free(prim);
		prim = next;
	}
	gcollect_garbage(state);
	free(state);
}

void gfile_append_to_top(gstate_t* state, FILE* in)
{
	FILE* inp = ginput_stream;
	ginput_stream = in;
	gread_token();
	gexpr_t* head = gexpression();
	gexpr_t* last = head;
	while(last != NULL)
	{
		gexpr_t* exp = gexpression();
		last->next = exp;
		last = exp;
	}
	last->next = NULL;
	gexpr_t* program_head = state->program_head;
	state->program_head = head;
	gexecute_program(state);
	last->next = program_head;
	ginput_stream = inp;
}

void gfile_load(gstate_t* state, FILE* in)
{
	ginput_stream = in;
	gtoken.lineno = 1;
	gread_token();
	state->program_head = gexpression();
	gexpr_t* last = state->program_head;
	while(last != NULL)
	{
		gexpr_t* exp = gexpression();
		last->next = exp;
		last = exp;
	}
}

void gfile_unload(gstate_t* state)
{
	ginput_stream = NULL;
	gexpr_t* prog_node = state->program_head;
	while(prog_node != NULL)
	{
		gexpr_t* next = prog_node->next;
		gdestroy_expr(prog_node);
		prog_node = next;
	}
	state->program_head = NULL;
}

gsymbol_t* gsymbol_get(gstate_t* state, char* name)
{
	long symstack_size = state->symbols_stack_size;
	gsymbol_t* sym;
	while(symstack_size >= 0)
	{
		sym = state->symbols_stack[symstack_size];
		while(sym != NULL)
		{
			if(strcmp(sym->name, name) == 0)
				return sym;
			sym = sym->next;
		}
		--symstack_size;
	}
	gfatal_error("attempted to access non-existent symbol (%s)\n", name);
	return NULL;
}

gsymbol_t* gsymbol_create(gstate_t* state, char* name)
{
	gsymbol_t* sym = malloc(sizeof(gsymbol_t));
	if(!sym) gfatal_error("out of memory\n");
	sym->name = name;
	sym->value = NULL;
	sym->next = state->symbols_stack[state->symbols_stack_size];
	state->symbols_stack[state->symbols_stack_size] = sym;
	return sym;
}

void gadd_primitive(gstate_t* state, const char* name, void (*fn)(struct gstate*))
{
	gprimitive_t* prim = malloc(sizeof(gprimitive_t));
	if(!prim) gfatal_error("out of memory\n");
	prim->name = name;
	prim->fn = fn;
	prim->next = state->primitives_head;
	state->primitives_head = prim;
}

void gadd_primitivelib(gstate_t* state, gprimitivereg_t prims[])
{
	long i = 0;
	while(1)
	{
		if(prims[i].fn == NULL) break;
		gadd_primitive(state, prims[i].name, prims[i].fn);
		++i;
	}
}

gprimitive_t* gprimitive_get(gstate_t* state, const char* name)
{
	gprimitive_t* prim = state->primitives_head;
	while(prim)
	{
		if(strcmp(prim->name, name) == 0)
			return prim;
		prim = prim->next;
	}
	return NULL;
}

static gexpr_t* find_define_in_block(gexpr_t* exp, const char* name)
{
	gexpr_t* node = exp->blockexpr.block_head;
	
	while(node)
	{
		if(node->type == EXPR_DEFINE)
		{
			if(strcmp(node->defexpr.name, name) == 0)
				return node;
				
			gexpr_t* in_block = find_define_in_block(node->defexpr.block, name);
			if(in_block)
				return in_block;
		}
		node = node->next;
	}
	
	return NULL;
}

gexpr_t* gdefine_get(gstate_t* state, const char* name)
{
	gexpr_t* prog_node = state->program_head;
	while(prog_node)
	{
		if(prog_node->type == EXPR_DEFINE)
		{
			if(strcmp(prog_node->defexpr.name, name) == 0)
				return prog_node;
			
			gexpr_t* in_block = find_define_in_block(prog_node->defexpr.block, name);
			if(in_block)
				return in_block;
		}
		prog_node = prog_node->next;
	}
	return NULL;
}

void gsymbol_setval(gsymbol_t* sym, gobject_t* value)
{
	sym->value = value;
}

void gsymbol_push(gstate_t* state)
{
	if(state->symbols_stack_size == MAX_STACK_LEN - 1) gfatal_error("symbols stack overflow\n");
	++state->symbols_stack_size;
	state->symbols_stack[state->symbols_stack_size] = NULL;
}

void gsymbol_pop(gstate_t* state)
{
	if(state->symbols_stack_size == 0) gfatal_error("symbol stack underflow\n");
	gsymbol_t* sym = state->symbols_stack[state->symbols_stack_size];
	while(sym)
	{
		gsymbol_t* next = sym->next;
		free(sym);
		sym = next;
	}
	--state->symbols_stack_size;
}

