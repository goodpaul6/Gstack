/* -- gstdlib.c -- a stack based programming language */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gstack.h"

#define HASH_TABLE_ENTRIES_AMT		2048
#define MAX_CONVERTED_STRING_LEN	128

void add_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value + n2->number.value);
}

void sub_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value - n2->number.value);
}

void mul_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value * n2->number.value);
}

void div_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value / n2->number.value);
}

void cons_prim(gstate_t* state)
{
	gpush_pair(state);
}

static void print_obj(gobject_t* obj)
{
	switch(obj->type)
	{
	case OBJ_NUMBER:
		printf("%f\n", obj->number.value);
		break;
	case OBJ_BOOLEAN:
		printf("%s\n", obj->boolean.value ? "true" : "false");
		break;
	case OBJ_STRING:
		printf("%s\n", obj->string.value);
		break;
	case OBJ_PAIR:
		print_obj(obj->pair.head);
		print_obj(obj->pair.tail);
		break;
	case OBJ_NATIVE:
		printf("native pointer\n"); 
		break;
	}
}

void println_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	print_obj(obj);
}

void putchar_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	putchar((char)obj->number.value);
}

void dup_prim(gstate_t* state)
{
	gobject_t* obj = gpeek_object(state);
	gpush_object(state, obj);
}

void drop_prim(gstate_t* state)
{
	gpop_object(state);
}

static char are_objects_equal(gobject_t* o1, gobject_t* o2)
{
	if(o1->type != o2->type) return 0;
	
	switch(o1->type)
	{
	case OBJ_NUMBER:
		return o1->number.value == o2->number.value;
	case OBJ_BOOLEAN:
		return o1->boolean.value == o2->boolean.value;
	case OBJ_STRING:
		return (strcmp(o1->string.value, o2->string.value) == 0);
	case OBJ_PAIR:
		return (are_objects_equal(o1->pair.head, o2->pair.head) && are_objects_equal(o1->pair.tail, o2->pair.tail));
	case OBJ_NATIVE:
		return o1->native.value == o2->native.value;
	}
	return 0;
}

void exit_prim(gstate_t* state)
{	
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	int v = (int)obj->number.value;
	
	gfile_unload(state);
	gdestroy_state(state);
	exit(v);
}

void equals_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_object(state);
	gobject_t* o1 = gpop_object(state);
	gpush_boolean(state, are_objects_equal(o1, o2));
}

void head_prim(gstate_t* state)
{
	gobject_t* pair = gpeek_expect(state, OBJ_PAIR);
	gpush_object(state, pair->pair.head);
}

void tail_prim(gstate_t* state)
{
	gobject_t* pair = gpeek_expect(state, OBJ_PAIR);
	gpush_object(state, pair->pair.tail);
}

void gc_prim(gstate_t* state)
{
	gcollect_garbage(state);
}

void show_prim(gstate_t* state)
{
	long stack_loc = state->stack_size - 1;
	while(stack_loc >= 0)
	{	
		printf("stack location %ld\n----------------\n", stack_loc);
		print_obj(state->stack[stack_loc--]);
	}
}

void getchar_prim(gstate_t* state)
{
	gpush_number(state, getchar());
}

void gt_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value > o2->number.value);
}

void lt_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value < o2->number.value);
}

void gte_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value >= o2->number.value);
}

void lte_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value <= o2->number.value);
}

void floor_prim(gstate_t* state)
{
	gobject_t* num = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, (int)num->number.value);
}

void ceil_prim(gstate_t* state)
{
	gobject_t* num = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, (int)num->number.value + 1);
}

void mod_prim(gstate_t* state)
{
	gobject_t* modval = gpop_expect(state, OBJ_NUMBER);
	gobject_t* num = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, (int)num->number.value % (int)modval->number.value); 
}

void file_on_gc(void** value)
{
	fclose((FILE*)(*value));
	(*value) = NULL;
}

void file_open_prim(gstate_t* state)
{
	gobject_t* mode = gpop_expect(state, OBJ_STRING);
	gobject_t* path = gpop_expect(state, OBJ_STRING);
	FILE* file = fopen(path->string.value, mode->string.value);
	if(!file) gfatal_error("file open (%s with mode %s) failed\n", path->string.value, mode->string.value);
	gpush_native(state, file, NULL, NULL, file_on_gc);
}

void file_getchar_prim(gstate_t* state)
{
	gobject_t* file = gpop_expect(state, OBJ_NATIVE);
	FILE* nfile = (FILE*)file->native.value;
	gpush_number(state, fgetc(nfile));
}

typedef struct nat_list
{
	size_t capacity;
	size_t length;
	gobject_t** items;
} nat_list_t;

static void list_realloc(nat_list_t* list)
{
	gobject_t** new_items = realloc(list->items, list->capacity * sizeof(gobject_t*));
	if(!new_items) gfatal_error("out of memory\n");
	list->items = new_items;
}

void list_on_mark(void** value)
{
	nat_list_t* list = (nat_list_t*)(*value);
	size_t i; for (i = 0; i < list->length; i++)
		gmark_object(list->items[i]);
}

void list_on_gc(void** value)
{
	nat_list_t* list = (nat_list_t*)(*value);
	free(list->items);
	free(list);
	*value = NULL;
}

void list_create_prim(gstate_t* state)
{
	nat_list_t* list = malloc(sizeof(nat_list_t));
	if(!list) gfatal_error("out of memory\n");
	list->capacity = 2;
	list->length = 0;
	list->items = NULL;
	list_realloc(list);
	gpush_native(state, list, list_on_mark, NULL, list_on_gc);
}

static void list_extend(nat_list_t* list);

static void list_reverse(nat_list_t* list)
{
	size_t i; for(i = 0; i < list->length / 2; i++)
	{
		gobject_t* temp = list->items[i];
		list->items[i] = list->items[list->length - i - 1];
		list->items[list->length - i - 1] = temp;
	}
}

static void list_additem(nat_list_t* list, gobject_t* obj)
{
	list->length += 1;
	list_extend(list);
	list->items[list->length - 1] = obj;
}

void list_createblock_prim(gstate_t* state)
{
	gobject_t* size = gpop_expect(state, OBJ_NUMBER);
	nat_list_t* list = malloc(sizeof(nat_list_t));
	if(!list) gfatal_error("out of memory\n");
	size_t amt = (size_t)size->number.value;
	list->capacity = amt * 2;
	list->length = amt;
	list->items = NULL;
	list_realloc(list);
	size_t i; for(i = 0; i < amt; i++)
		list->items[amt - i - 1] = gpop_object(state);
	gpush_native(state, list, list_on_mark, NULL, list_on_gc);
}

void list_reverse_prim(gstate_t* state)
{
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	list_reverse(list);
}

static void list_extend(nat_list_t* list)
{
	while(list->length >= list->capacity) 
	{
		list->capacity *= 2;
		list_realloc(list);
	}
}

void list_append_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	list_additem(list, obj);	
}

void list_get_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	if((long)obj->number.value >= list->length) gfatal_error("list index is out of bounds (%ld)\n", (long)obj->number.value); 
	gpush_object(state, list->items[(long)obj->number.value]);
}

void list_set_prim(gstate_t* state)
{
	gobject_t* idxobj = gpop_expect(state, OBJ_NUMBER);
	gobject_t* obj = gpop_object(state);
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	long idx = (long)(idxobj->number.value);
	if(idx >= list->length) gfatal_error("list index is out of bounds (%ld)\n", (long)obj->number.value);
	list->items[idx] = obj;
}

void list_depend_prim(gstate_t* state)
{
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	if(list->length == 0) gfatal_error("cannot depend from empty list\n");
	--list->length;
}

void list_length_prim(gstate_t* state)
{
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);	
	gpush_number(state, list->length);
}

static void list_remove_idx(nat_list_t* list, long idx)
{
	if(idx >= list->length) gfatal_error("attempted to remove object from non-existent index in list (%ld)\n", idx);
	if(idx == list->length - 1) 
	{
		list->length -= 1;
		return;
	}

	memmove(list->items + idx, list->items + (idx + 1), sizeof(gobject_t*) * (list->length - idx - 1));
	list->length -= 1;
}

void list_remove_prim(gstate_t* state)
{ 
	gobject_t* obj = gpop_object(state);
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);	
	long i; for(i = 0; i < list->length; i++)
	{
		if(list->items[i] == obj)
		{
			list_remove_idx(list, i);
			return;
		}
	}
	gfatal_error("could not find object in list, removal failed\n");
}

void list_resize_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	long val = (long)obj->number.value;
	if(val < 0) gfatal_error("cannot resize list to value lower than 0\n");
	list->length = val;
	list_extend(list);
}

void list_clear_prim(gstate_t* state)
{
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	list->length = 0;
}

void io_puts_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_STRING);
	if(!printf("%s", obj->string.value)) gfatal_error("io.puts failed\n");
}

void io_putn_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	if(!printf("%f", obj->number.value)) gfatal_error("io.putn failed\n");
}

void io_putnp_prim(gstate_t* state)
{
	gobject_t* prec = gpop_expect(state, OBJ_NUMBER);
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	if(!printf("%.*f", (int)prec->number.value, obj->number.value)) gfatal_error("io.putnp failed\n");
}

void io_putb_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_BOOLEAN);
	if(!printf("%s", obj->boolean.value ? "true" : "false")) gfatal_error("io.putb failed\n");
}

typedef struct nat_sbuf
{
	size_t capacity;
	size_t length;
	char* str;
} nat_sbuf_t;

static void sbuf_realloc(nat_sbuf_t* sbuf)
{
	char* new_buf = realloc(sbuf->str, sbuf->capacity * sizeof(char));
	if(!new_buf) gfatal_error("out of memory\n");
	sbuf->str = new_buf;
}

void sbuf_on_gc(void** value)
{
	nat_sbuf_t* sbuf = (nat_sbuf_t*)(*value);
	free(sbuf->str);
	free(sbuf);
	*value = NULL;
}

static void sbuf_extend(nat_sbuf_t* sbuf)
{	
	while(sbuf->length + 1 >= sbuf->capacity)
	{
		sbuf->capacity *= 2;
		sbuf_realloc(sbuf);
	}
}

void sbuf_create_prim(gstate_t* state)
{
	nat_sbuf_t* sbuf = malloc(sizeof(nat_sbuf_t));
	sbuf->capacity = 2;
	sbuf->length = 0;
	sbuf->str = NULL;
	sbuf_realloc(sbuf);
	gpush_native(state, sbuf, NULL, NULL, sbuf_on_gc);
}

static void append_char(nat_sbuf_t* sbuf, char c)
{
	sbuf->length += 1;
	sbuf_extend(sbuf);
	sbuf->str[sbuf->length - 1] = c;
	sbuf->str[sbuf->length] = '\0';
}

void sbuf_append_char_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	char c = (char)obj->number.value;
	nat_sbuf_t* sbuf = (nat_sbuf_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	append_char(sbuf, c);
}

void sbuf_append_str_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_STRING);
	nat_sbuf_t* sbuf = (nat_sbuf_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	long i; for(i = 0; i < obj->string.length; i++)
		append_char(sbuf, obj->string.value[i]);
}

void sbuf_clear_prim(gstate_t* state)
{
	nat_sbuf_t* sbuf = (nat_sbuf_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	sbuf->str[0] = '\0';
	sbuf->length = 0;
}

void sbuf_tostring_prim(gstate_t* state)
{
	nat_sbuf_t* sbuf = (nat_sbuf_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	gpush_string(state, strdup(sbuf->str), sbuf->length, 0);
}

void tonumber_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	float number;
	
	switch(obj->type)
	{
		case OBJ_NUMBER:
			number = obj->number.value;
			break;
		case OBJ_BOOLEAN:
			number = obj->boolean.value ? 1 : 0;
			break;
		case OBJ_STRING:
			number = (float)strtod(obj->string.value, NULL);
			break;
		case OBJ_PAIR:
			gfatal_error("cannot convert pair to number\n");
			break;
		case OBJ_NATIVE:
			gfatal_error("cannot convert native object to number\n");
			break;
	}
	
	gpush_number(state, number);
}

typedef struct nat_hash_entry
{
	char* key;
	gobject_t* value;					
	struct nat_hash_entry* next;		
} nat_hash_entry_t;

typedef struct nat_hash_table
{
	nat_hash_entry_t* entries[HASH_TABLE_ENTRIES_AMT];
} nat_hash_table_t;

static size_t hash_function(char* key)
{
	char* str = key;
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

void hash_table_on_mark(void** value)
{
	nat_hash_table_t* table = (nat_hash_table_t*)(*value);
	long i; for(i = 0; i < HASH_TABLE_ENTRIES_AMT; i++)
	{
		nat_hash_entry_t* entry = table->entries[i];
		while(entry)
		{
			gmark_object(entry->value);
			entry = entry->next;
		}
	}
}

void hash_table_on_gc(void** value)
{
	nat_hash_table_t* table = (nat_hash_table_t*)(*value);
	long i; for(i = 0; i < HASH_TABLE_ENTRIES_AMT; i++)
	{
		nat_hash_entry_t* entry = table->entries[i];
		while(entry)
		{
			nat_hash_entry_t* next = entry->next;
			free(entry->key);
			free(entry);
			entry = next;
		}
	}
	free(*value);
}

void hash_table_create_prim(gstate_t* state)
{
	nat_hash_table_t* table = calloc(1, sizeof(nat_hash_table_t));
	if(!table) gfatal_error("out of memory\n");
	gpush_native(state, table, hash_table_on_mark, NULL, hash_table_on_gc);
}

void hash_table_put_prim(gstate_t* state)
{
	gobject_t* value = gpop_object(state);
	gobject_t* key = gpop_expect(state, OBJ_STRING);
	nat_hash_table_t* table = (nat_hash_table_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	
	char* keydup = strdup(key->string.value);
	nat_hash_entry_t* entry = calloc(1, sizeof(nat_hash_entry_t));
	if(!entry) gfatal_error("out of memory\n");
	entry->key = keydup;
	entry->value = value;
	size_t idx = hash_function(keydup) % HASH_TABLE_ENTRIES_AMT;
	entry->next = table->entries[idx];
	table->entries[idx] = entry;
}

void hash_table_get_prim(gstate_t* state)
{
	gobject_t* key = gpop_expect(state, OBJ_STRING);
	nat_hash_table_t* table = (nat_hash_table_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);

	size_t idx = hash_function(key->string.value) % HASH_TABLE_ENTRIES_AMT;

	nat_hash_entry_t* entry = table->entries[idx];
	
	while(entry)
	{
		if(strcmp(key->string.value, entry->key) == 0)
		{
			gpush_object(state, entry->value);
			return;
		}
		entry = entry->next;
	}
	gfatal_error("attempted to access non-existent key in hash table (%s)\n", key->string.value);
}

void hash_table_exists_prim(gstate_t* state)
{
	gobject_t* key = gpop_expect(state, OBJ_STRING);
	nat_hash_table_t* table = (nat_hash_table_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);

	size_t idx = hash_function(key->string.value) % HASH_TABLE_ENTRIES_AMT;

	nat_hash_entry_t* entry = table->entries[idx];
	
	while(entry)
	{
		if(strcmp(key->string.value, entry->key) == 0)
		{
			gpush_boolean(state, 1);
			return;
		}
		entry = entry->next;
	}
	gpush_boolean(state, 0);
}

void hash_table_set_prim(gstate_t* state)
{
	gobject_t* value = gpop_object(state);
	gobject_t* key = gpop_expect(state, OBJ_STRING);
	nat_hash_table_t* table = (nat_hash_table_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);

	size_t idx = hash_function(key->string.value) % HASH_TABLE_ENTRIES_AMT;

	nat_hash_entry_t* entry = table->entries[idx];
	
	while(entry)
	{
		if(strcmp(key->string.value, entry->key) == 0)
		{
			entry->value = value;
			return;
		}
		entry = entry->next;
	}
}

void hash_table_remove_prim(gstate_t* state)
{
	gobject_t* key = gpop_expect(state, OBJ_STRING);
	nat_hash_table_t* table = (nat_hash_table_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);

	size_t idx = hash_function(key->string.value) % HASH_TABLE_ENTRIES_AMT;

	nat_hash_entry_t* entry = table->entries[idx];	
	while(entry)
	{
		if(strcmp(key->string.value, entry->key) == 0)
		{
			nat_hash_entry_t* next = entry->next;
			free(entry->key);
			free(entry);
			table->entries[idx] = next;
			return;
		}
		entry = entry->next;
	}
}

void tostring_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	
	char* buffer = malloc(sizeof(char) * MAX_CONVERTED_STRING_LEN);
	if(!buffer) gfatal_error("out of memory\n");
	long length = 0;
	
	switch(obj->type)
	{
		case OBJ_NUMBER:
			length = sprintf(buffer, "%f", obj->number.value);
			break;
		case OBJ_BOOLEAN:
			length = sprintf(buffer, "%s", obj->boolean.value ? "true" : "false");
			break;
		case OBJ_STRING:
			gfatal_error("attempted to convert string to a string, this is most likely an error\n");
			break;
		case OBJ_NATIVE:
			gfatal_error("cannot convert native object to string, maybe this native object has a specific function for that very purpose\n");
			break;
		case OBJ_PAIR:
			gfatal_error("cannot convert pair to string\n");
			break;
	}
	
	gpush_string(state, buffer, length, 0);
}

static char is_true(gobject_t* obj)
{
	if(obj->type == OBJ_NUMBER)
		return (obj->number.value != 0);
	if(obj->type == OBJ_BOOLEAN)
		return obj->boolean.value;
	return 1;
}

void and_prim(gstate_t* state)
{
	gobject_t* b2 = gpop_object(state);
	gobject_t* b1 = gpop_object(state);
	
	char val = is_true(b1) && is_true(b2);
	
	gpush_boolean(state, val);
}

void or_prim(gstate_t* state)
{
	gobject_t* b2 = gpop_object(state);
	gobject_t* b1 = gpop_object(state);
	
	char val = is_true(b1) || is_true(b2);
	
	gpush_boolean(state, val);
}

void not_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	gpush_boolean(state, !is_true(obj));
}

void bitwise_and_prim(gstate_t* state)
{
	gobject_t* sh = gpop_expect(state, OBJ_NUMBER);
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	
	unsigned long v1 = (unsigned long)obj->number.value;
	unsigned long v2 = (unsigned long)sh->number.value;
	
	gpush_number(state, v1 & v2);
}

void bitwise_or_prim(gstate_t* state)
{
	gobject_t* sh = gpop_expect(state, OBJ_NUMBER);
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	
	unsigned long v1 = (unsigned long)obj->number.value;
	unsigned long v2 = (unsigned long)sh->number.value;
	
	gpush_number(state, v1 | v2);
}

void bitwise_shift_left(gstate_t* state)
{
	gobject_t* sh = gpop_expect(state, OBJ_NUMBER);
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	
	unsigned long v1 = (unsigned long)obj->number.value;
	unsigned long v2 = (unsigned long)sh->number.value;
	
	gpush_number(state, v1 << v2);
}


void bitwise_shift_right(gstate_t* state)
{
	gobject_t* sh = gpop_expect(state, OBJ_NUMBER);
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	
	unsigned long v1 = (unsigned long)obj->number.value;
	unsigned long v2 = (unsigned long)sh->number.value;
	
	gpush_number(state, v1 >> v2);
}

void call_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_STRING);
	gcall_function(state, obj->string.value);
}

void type_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	gpush_number(state, obj->type);
}

typedef struct nat_closure
{
	gsymbol_t* symbols_stack[MAX_STACK_LEN];
	long symbols_stack_size;
} nat_closure_t;

static void mark_all_symbols(gsymbol_t* sym)
{
	while(sym)
	{
		gsymbol_mark(sym);
		sym = sym->next;
	}
}

static void closure_clear(nat_closure_t* clos)
{	
	long size = clos->symbols_stack_size;
	
	while(size >= 0)
	{
		gsymbol_t* next;
		gsymbol_t* sym; for(sym = clos->symbols_stack[size]; sym != NULL; sym = next)
		{
			next = sym->next;
			free(sym->name);
			free(sym);
		}
		--size;
	}
	
	clos->symbols_stack_size = 0;
	clos->symbols_stack[0] = NULL;
}

void closure_on_gc(void** value)
{
	nat_closure_t* clos = (nat_closure_t*)(*value);
	closure_clear(clos);
	free(clos);
	*value = NULL;
}

void closure_on_mark(void** value)
{
	nat_closure_t* clos = (nat_closure_t*)(*value);
	long size = clos->symbols_stack_size;
	
	while(size >= 0)
	{
		mark_all_symbols(clos->symbols_stack[size]);
		--size;
	}
}

void closure_create_prim(gstate_t* state)
{
	nat_closure_t* clos = calloc(1, sizeof(nat_closure_t));
	if(!clos) gfatal_error("out of memory\n");
	clos->symbols_stack_size = 0;
	clos->symbols_stack[0] = NULL;
	gpush_native(state, clos, closure_on_mark, NULL, closure_on_gc);
}

void closure_capture_prim(gstate_t* state)
{
	nat_closure_t* clos = (nat_closure_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	closure_clear(clos);
	
	clos->symbols_stack_size = state->symbols_stack_size;
	
	long pos = 0;
	while(pos <= state->symbols_stack_size)
	{
		gsymbol_t* sym = state->symbols_stack[pos];
		gsymbol_t* clos_sym = NULL;
		while(sym)
		{
			clos_sym = malloc(sizeof(gsymbol_t));
			if(!clos_sym) gfatal_error("out of memory\n");
			
			clos_sym->next = clos->symbols_stack[pos];
			clos->symbols_stack[pos] = clos_sym;
			
			clos_sym->name = strdup(sym->name);
			if(!clos_sym->name) gfatal_error("out of memory\n");
			clos_sym->value = sym->value;
			
			sym = sym->next;
		}
		++pos;
	}
}

static gsymbol_t* closure_get_sym(nat_closure_t* clos, const char* name)
{
	long size = clos->symbols_stack_size;
	
	while(size >= 0)
	{
		gsymbol_t* sym = clos->symbols_stack[size];
		while(sym)
		{
			if(strcmp(sym->name, name) == 0)
				return sym;
			sym = sym->next;
		}
		--size;
	}
	gfatal_error("attempted to get non-existent symbol from closure object (%s)\n", name);
	return NULL;
}

void closure_get_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_STRING);
	nat_closure_t* clos = (nat_closure_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	
	gsymbol_t* sym = closure_get_sym(clos, obj->string.value);
	gpush_object(state, sym->value);
}

void closure_set_prim(gstate_t* state)
{
	gobject_t* new_val = gpop_object(state);
	gobject_t* obj = gpop_expect(state, OBJ_STRING);
	nat_closure_t* clos = (nat_closure_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	
	gsymbol_t* sym = closure_get_sym(clos, obj->string.value);
	sym->value = new_val;
}

void newline_prim(gstate_t* state)
{
	if(!putchar('\n')) gfatal_error("newline failed\n");
}

void srand_prim(gstate_t* state)
{
	srand(time(NULL));
}

void rand_prim(gstate_t* state)
{
	gpush_number(state, (float)rand() / (float)RAND_MAX); 
}

void dofile_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_STRING);
	FILE* file = fopen(obj->string.value, "r");
	if(!file) gfatal_error("attempted to open file with name %s, failed\n", obj->string.value);
	const char* prev_errfile = gerrfilename;
	gerrfilename = obj->string.value;
	gfile_append_to_top(state, file);
	gerrfilename = prev_errfile;
	fclose(file);
}

static gprimitivereg_t standard_library[] =
{
	{"add", add_prim},
	{"sub", sub_prim},
	{"mul", mul_prim},
	{"div", div_prim},
	{"cons", cons_prim},
	{"debug.println", println_prim},
	{"putchar", putchar_prim},
	{"dup", dup_prim},
	{"drop", drop_prim},
	{"exit", exit_prim},
	{"equals", equals_prim},
	{"head", head_prim},
	{"tail", tail_prim},
	{"gc", gc_prim},
	{"show", show_prim},
	{"getchar", getchar_prim},
	{"gt", gt_prim},
	{"lt", lt_prim},
	{"gte", gte_prim},
	{"lte", lte_prim},
	{"floor", floor_prim},
	{"ceil", ceil_prim},
	{"mod", mod_prim},
	{"file.open", file_open_prim},
	{"file.getchar", file_getchar_prim},
	{"list.new", list_create_prim},
	{"list.newblock", list_createblock_prim},
	{"list.reverse", list_reverse_prim},
	{"list.get", list_get_prim},
	{"list.append", list_append_prim},
	{"list.depend", list_depend_prim},
	{"list.length", list_length_prim},
	{"list.set", list_set_prim},
	{"list.resize", list_resize_prim},
	{"list.clear", list_clear_prim},
	{"list.remove", list_remove_prim},
	{"io.puts", io_puts_prim},
	{"io.putn", io_putn_prim},
	{"io.putnp", io_putnp_prim}, 
	{"io.putb", io_putb_prim},
	{"strbuf.new", sbuf_create_prim},
	{"strbuf.append.char", sbuf_append_char_prim},
	{"strbuf.append.str", sbuf_append_str_prim},
	{"strbuf.char", sbuf_append_char_prim},
	{"strbuf.str", sbuf_append_str_prim},
	{"strbuf.clear", sbuf_clear_prim},
	{"strbuf.tostring", sbuf_tostring_prim},
	{"hash.new", hash_table_create_prim},
	{"hash.put", hash_table_put_prim},
	{"hash.exists", hash_table_exists_prim},
	{"hash.get", hash_table_get_prim},
	{"hash.set", hash_table_set_prim},
	{"hash.remove", hash_table_remove_prim},
	{"tonumber", tonumber_prim},
	{"tostring", tostring_prim},
	{"and", and_prim},
	{"or", or_prim},
	{"not", not_prim},
	{"bit.and", bitwise_and_prim},
	{"bit.or", bitwise_or_prim},
	{"bit.asl", bitwise_shift_left},
	{"bit.asr", bitwise_shift_right}, 
	{"call", call_prim},
	{"typeof", type_prim},
	{"closure.new", closure_create_prim},
	{"closure.capture", closure_capture_prim},
	{"closure.get", closure_get_prim},
	{"closure.set", closure_set_prim},
	{"newline", newline_prim},
	{"random.seed", srand_prim},
	{"random.float", rand_prim},
	{"dofile", dofile_prim},
	{"", NULL}
};

void gload_stdlib(gstate_t* state)
{
	gadd_primitivelib(state, standard_library);
	GUTIL_ADDCONST(state, "number", OBJ_NUMBER);
	GUTIL_ADDCONST(state, "boolean", OBJ_BOOLEAN);
	GUTIL_ADDCONST(state, "string", OBJ_STRING);
	GUTIL_ADDCONST(state, "pair", OBJ_PAIR);
	GUTIL_ADDCONST(state, "native", OBJ_NATIVE);
}
