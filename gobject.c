/* -- gobject.c -- a stack based programming language */
#include <stdlib.h>
#include "gstack.h"

gobject_t* gmake_object(gstate_t* state, gobject_type_t type)
{
	if(state->object_amt == state->max_object_amt) gcollect_garbage(state);
	
	gobject_t* obj = malloc(sizeof(gobject_t));
	if(!obj) gfatal_error("out of memory\n");
	obj->marked = 0;
	obj->type = type;
	obj->next = state->gc_head;
	state->gc_head = obj;
	++state->object_amt;
	return obj;
}

void gdestroy_object(gobject_t* obj)
{
	if(!obj) return;
	if(obj->type == OBJ_STRING)
	{
		if(!obj->string.is_lit)
			free(obj->string.value);
	}
	free(obj);
}

void gpush_object(gstate_t* state, gobject_t* obj)
{
	if(state->stack_size == MAX_STACK_LEN) gfatal_error("stack overflow at size %ld\n", (long)MAX_STACK_LEN);
	state->stack[state->stack_size++] = obj;
}

gobject_t* gpop_object(gstate_t* state)
{
	if(state->stack_size == 0) gfatal_error("stack underflow\n");
	return state->stack[--state->stack_size];
}

gobject_t* gpop_expect(gstate_t* state, gobject_type_t type)
{
	gobject_t* obj = gpop_object(state);
	if(obj->type != type) gfatal_error("expected object of type %i on stack, but found object of type %i\n", type, obj->type);
	return obj;
}

gobject_t* gpeek_object(gstate_t* state)
{
	if(state->stack_size == 0) gfatal_error("stack underflow (peek)\n");
	return state->stack[state->stack_size - 1];
}

gobject_t* gpeek_expect(gstate_t* state, gobject_type_t type)
{
	gobject_t* obj = gpeek_object(state);
	if(obj->type != type) gfatal_error("expected object of type %i on stack, but found object of type %i\n", type, obj->type);
	return obj;
}

void gpush_number(gstate_t* state, float value)
{
	gobject_t* obj = gmake_object(state, OBJ_NUMBER);
	obj->number.value = value;
	gpush_object(state, obj);
}

void gpush_boolean(gstate_t* state, char value)
{
	gobject_t* obj = gmake_object(state, OBJ_BOOLEAN);
	obj->boolean.value = value;
	gpush_object(state, obj);
}

void gpush_string(gstate_t* state, char* value, long length, char is_literal)
{
	gobject_t* obj = gmake_object(state, OBJ_STRING);
	obj->string.value = value;
	obj->string.length = length;
	obj->string.is_lit = is_literal;
	gpush_object(state, obj);
}

void gpush_pair(gstate_t* state)
{
	gobject_t* obj = gmake_object(state, OBJ_PAIR);
	gobject_t* tail = gpop_object(state);
	gobject_t* head = gpop_object(state);
	obj->pair.head = head;
	obj->pair.tail = tail;
	gpush_object(state, obj);
}

void gpush_native(struct gstate* state, void* value, void (*on_mark)(void** val), void (*on_unmark)(void** val), void (*on_gc)(void** val))
{
	gobject_t* obj = gmake_object(state, OBJ_NATIVE);
	obj->native.value = value;
	obj->native.on_mark = on_mark;
	obj->native.on_unmark = on_unmark;
	obj->native.on_gc = on_gc;
	gpush_object(state, obj);
}
