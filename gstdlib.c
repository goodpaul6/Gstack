/* -- gstdlib.c -- a stack based programming language */
#include <stdlib.h>
#include <string.h>
#include "gstack.h"

#define HASH_TABLE_ENTRIES_AMT	2048

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
	if(o1->type != o2->type) gfatal_error("attempted to compare equality between 2 objects of different types (%i and %i)\n", o1->type, o2->type);
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

void list_append_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	list->length += 1;
	while(list->length >= list->capacity) 
	{
		list->capacity *= 2;
		list_realloc(list);
	}
	list->items[list->length - 1] = obj;
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

void list_clear_prim(gstate_t* state)
{
	nat_list_t* list = (nat_list_t*)(gpop_expect(state, OBJ_NATIVE)->native.value);
	list->length = 0;
}

void io_puts_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_STRING);
	if(!puts(obj->string.value)) gfatal_error("io.puts failed\n");
}

void io_putn_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	if(!printf("%f", obj->number.value)) gfatal_error("io.putn failed\n");
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
	{"list.get", list_get_prim},
	{"list.append", list_append_prim},
	{"list.depend", list_depend_prim},
	{"list.length", list_length_prim},
	{"list.set", list_set_prim},
	{"list.clear", list_clear_prim},
	{"io.puts", io_puts_prim},
	{"io.putn", io_putn_prim}, 
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
	{"hash.get", hash_table_get_prim},
	{"hash.set", hash_table_set_prim},
	{"hash.remove", hash_table_remove_prim},
	{"tonumber", tonumber_prim}, 
	{"", NULL}
};

void gload_stdlib(gstate_t* state)
{
	gadd_primitivelib(state, standard_library);
}
