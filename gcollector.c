/* -- gcollector.c -- a stack based programming language */
#include <stdlib.h>
#include "gstack.h"

void gmark_object(gobject_t* obj)
{
	if(!obj) return;
	if(obj->marked) return;
	
	obj->marked = 1;
	
	if(obj->type == OBJ_PAIR)
	{
		gmark_object(obj->pair.head);
		gmark_object(obj->pair.tail);
	}
	
	if(obj->type == OBJ_NATIVE)
	{
		if(obj->native.on_mark)
			obj->native.on_mark(&obj->native.value);
	}
}

static void mark_symbol(gsymbol_t* sym)
{
	if(!sym) return;
	gmark_object(sym->value);
}

static void mark_all(gstate_t* state)
{
	long i; for(i = 0; i < state->stack_size; i++)
		gmark_object(state->stack[i]);
	
	long symstack_size = state->symbols_stack_size;
	while(symstack_size >= 0)
	{
		gsymbol_t* sym;
		for(sym = state->symbols_stack[symstack_size]; sym != NULL; sym = sym->next)
			mark_symbol(sym);
		--symstack_size;
	}
}

static void sweep(gstate_t* state)
{
	gobject_t** object = &state->gc_head;
	while(*object)
	{
		if(!(*object)->marked)
		{
			gobject_t* unreached = *object;
			if(unreached->type == OBJ_NATIVE) 
			{
				if(unreached->native.on_gc)
					unreached->native.on_gc(&unreached->native.value);
			}
			*object = unreached->next;
			gdestroy_object(unreached);
			--state->object_amt;
		}
		else
		{
			(*object)->marked = 0;
			if((*object)->type == OBJ_NATIVE) 
			{
				if((*object)->native.on_unmark)
					(*object)->native.on_unmark(&(*object)->native.value);
			}
			object = &(*object)->next;
		}
	}
}

void gcollect_garbage(gstate_t* state)
{
	long num_objects = state->object_amt;
	mark_all(state);
	sweep(state);
	state->max_object_amt = num_objects * 2;
}
