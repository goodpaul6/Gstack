/* -- gstack.h -- a stack based programming language */
#ifndef GSTACK_H_
#define GSTACK_H_

#include <stdio.h>

#define MAX_TOKEN_LEN		1024
#define	MAX_STACK_LEN		9000
#define INITIAL_GC_THRES	8

#define GUTIL_ADDCONST(state, name, value)	\
do   								\
{ 									\
	gsymbol_t* sym = gsymbol_create(state, name); \
	gpush_number(state, value);							\
	gsymbol_setval(sym, gpop_object(state));	\
} while (0)							\

void gfatal_error(const char* format, ...);

typedef enum
{
	TOK_NUMBER,
	TOK_BOOLEAN,
	TOK_STRING,
	TOK_SYMBOL,
	TOK_CALL,
	TOK_DEFINE,
	TOK_WHILE,
	TOK_IF,
	TOK_ELSE,
	TOK_OPENBLOCK,
	TOK_CLOSEBLOCK,
	TOK_OPENPAIR,
	TOK_CLOSEPAIR,
	TOK_OPENLIST,
	TOK_CLOSELIST,
	TOK_SET,
	TOK_CREATE,
	TOK_EOF
} gtoken_type_t;

FILE* ginput_stream;

struct
{
	gtoken_type_t type;
	char buffer[MAX_TOKEN_LEN];
	long length;
	
	union
	{
		char boolean;
		float number;
	};
} gtoken;

void gread_token();
void gpeek_token();

typedef enum
{
	EXPR_NUMBER,
	EXPR_BOOLEAN,
	EXPR_STRING,
	EXPR_PAIR,
	EXPR_SYMBOL,
	EXPR_CALL,
	EXPR_DEFINE,
	EXPR_WHILE,
	EXPR_IF,
	EXPR_BLOCK,
	EXPR_SET,
	EXPR_CREATE,
	EXPR_LIST,
} gexpr_type_t;

typedef struct gexpr
{
	gexpr_type_t type;
	struct gexpr* next;
	
	union
	{
		struct { float value; } numexpr;
		struct { char value; } boolexpr;
		struct { char* value; long length; } strexpr;
		struct { char* value; } symexpr;
		struct { char* value; } callexpr;
		struct { char* name; struct gexpr* block; } defexpr;
		struct { struct gexpr* cond; struct gexpr* block; } whileexpr;
		struct { struct gexpr* cond; struct gexpr* true_expr; struct gexpr* false_expr; } ifexpr;
		struct { struct gexpr* block_head; } blockexpr;
		struct { char* name; } setexpr;
		struct { char* name; } createexpr;
		struct { struct gexpr* head; struct gexpr* tail; } pairexpr;
		struct { char* name; } defrefexpr;
		struct { struct gexpr* exprs_head; } listexpr;
	};
} gexpr_t;

gexpr_t* gmake_expr(gexpr_type_t type);

gexpr_t* gexpr_number();
gexpr_t* gexpr_boolean();
gexpr_t* gexpr_string();
gexpr_t* gexpr_pair();
gexpr_t* gexpr_symbol();
gexpr_t* gexpr_call();
gexpr_t* gexpr_define();
gexpr_t* gexpr_while();
gexpr_t* gexpr_if();
gexpr_t* gexpr_block();
gexpr_t* gexpr_set();
gexpr_t* gexpr_create();
gexpr_t* gexpr_list();

gexpr_t* gexpression();

void gdestroy_expr(gexpr_t* exp);
void gdebug_expr(gexpr_t* exp);

typedef enum
{
	OBJ_NUMBER,
	OBJ_BOOLEAN,
	OBJ_STRING,
	OBJ_PAIR,
	OBJ_NATIVE,
} gobject_type_t;

typedef struct gobject
{
	gobject_type_t type;
	char marked;
	struct gobject* next;
	
	union
	{
		struct { float value; } number;
		struct { char value; } boolean;
		struct { char* value; long length; char is_lit; } string;
		struct { struct gobject* head; struct gobject* tail; } pair;
		struct { void* value; void (*on_mark)(void** val); void (*on_unmark)(void** val); void (*on_gc)(void** val); } native;
	};
} gobject_t;

struct gstate;

gobject_t* gmake_object(struct gstate* state, gobject_type_t type);
void gdestroy_object(gobject_t* obj);

void gpush_object(struct gstate* state, gobject_t* obj);
gobject_t* gpop_object(struct gstate* state);
gobject_t* gpop_expect(struct gstate* state, gobject_type_t type);
gobject_t* gpeek_object(struct gstate* state);
gobject_t* gpeek_expect(struct gstate* state, gobject_type_t type);

void gpush_number(struct gstate* state, float value);
void gpush_boolean(struct gstate* state, char value);
void gpush_string(struct gstate* state, char* value, long length, char is_literal);
void gpush_pair(struct gstate* state);
void gpush_native(struct gstate* state, void* value, void (*on_mark)(void** val), void (*on_unmark)(void** val), void (*on_gc)(void** val));

void gmark_object(gobject_t* obj);
void gcollect_garbage(struct gstate* state);

typedef struct gprimitive
{
	struct gprimitive* next;
	const char* name;
	void (*fn)(struct gstate*);
} gprimitive_t;

typedef struct gprimitivereg
{
	const char* name;
	void (*fn)(struct gstate*);
} gprimitivereg_t;

typedef struct gsymbol
{
	struct gsymbol* next;
	char* name;
	gobject_t* value;
} gsymbol_t;

typedef struct gstate
{
	long object_amt;
	long max_object_amt;
	gsymbol_t* symbols_stack[MAX_STACK_LEN];
	long symbols_stack_size;
	gprimitive_t* primitives_head;
	gobject_t* gc_head;
	gobject_t* stack[MAX_STACK_LEN];
	long stack_size;
	gexpr_t* program_head;
} gstate_t;

gstate_t* gmake_state();
void gdestroy_state(gstate_t* state);

void gfile_load(gstate_t* state, FILE* in);
void gfile_unload(gstate_t* state);

void gsymbol_push(gstate_t* state);
gsymbol_t* gsymbol_get(gstate_t* state, char* name);
gsymbol_t* gsymbol_create(gstate_t* state, char* name);
void gsymbol_mark(gsymbol_t* sym);
void gsymbol_setval(gsymbol_t* sym, gobject_t* value);
void gadd_primitive(gstate_t* state, const char* name, void (*fn)(struct gstate*));
void gadd_primitivelib(gstate_t* state, gprimitivereg_t prims[]);
gprimitive_t* gprimitive_get(gstate_t* state, const char* name);
gexpr_t* gdefine_get(gstate_t* state, const char* name);
void gsymbol_pop(gstate_t* state);

void gcall_function(gstate_t* state, const char* name);
void gnode_do(gstate_t* state, gexpr_t* node);
void gexecute_program(gstate_t* state);

void gload_stdlib(gstate_t* state);

#endif
