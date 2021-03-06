/* -- glexer.c -- a stack based programming language */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "gstack.h"

static void geat_whitespace(FILE* in)
{
	int c;
	c = fgetc(in);
	
	if(isspace(c))
	{
		while(isspace(c))
		{	
			if(c == '\n') ++gtoken.lineno;
			c = fgetc(in);
		}
	}
	
	if(c != EOF)
		ungetc(c, in);
}

static int escape_seq(char c)
{
	switch(c)
	{
		case 'n':
			return '\n';
		case '0':
			return '\0';
		case 't':
			return '\t';
		case '\'':
			return '\'';
		case '"':
			return '\"';
		case '\\':
			return '\\';
	}
	gfatal_error("invalid escape sequence ('\\%c')\n", c);
	return 0;
}

static int peek(FILE* in)
{
	int c = fgetc(in);
	ungetc(c, in);
	return c;
}

void gread_token()
{
	FILE* in = ginput_stream;
	geat_whitespace(in);
	
	int c;
	long i;
	c = fgetc(in);
	
	if(isalpha(c) || c == '_')
	{
		i = 0;
		while(isalnum(c) || c == '_' || c == '.')
		{
			if(i < MAX_TOKEN_LEN - 1)
				gtoken.buffer[i++] = c;
			else
				gfatal_error("symbol exceeded maximum token length (%ld)\n", MAX_TOKEN_LEN);
			c = fgetc(in);
		}
		ungetc(c, in);
		gtoken.buffer[i] = '\0';
		gtoken.type = TOK_SYMBOL;
		
		if(strcmp(gtoken.buffer, "define") == 0)
			gtoken.type = TOK_DEFINE;
		if(strcmp(gtoken.buffer, "if") == 0)
			gtoken.type = TOK_IF;
		if(strcmp(gtoken.buffer, "else") == 0)
			gtoken.type = TOK_ELSE;
		if(strcmp(gtoken.buffer, "while") == 0)
			gtoken.type = TOK_WHILE;
		if(strcmp(gtoken.buffer, "true") == 0)
		{	
			gtoken.type = TOK_BOOLEAN;
			gtoken.boolean = 1;
		}
		if(strcmp(gtoken.buffer, "false") == 0)
		{
			gtoken.type = TOK_BOOLEAN;
			gtoken.boolean = 0;
		}
		if(strcmp(gtoken.buffer, "set") == 0)
			gtoken.type = TOK_SET;
		if(strcmp(gtoken.buffer, "create") == 0)
			gtoken.type = TOK_CREATE;
		
		gtoken.length = i;
	}
	else if(c == '0' && peek(in) == 'x')
	{
		i = 0;
		c = fgetc(in);
		c = fgetc(in);
		while(isxdigit(c))
		{
			if(i < MAX_TOKEN_LEN - 1)
				gtoken.buffer[i++] = c;
			else
				gfatal_error("hexadecimal digit length is greater than maximum token length\n");
			c = fgetc(in);
		}
		ungetc(c, in);
		gtoken.buffer[i] = '\0';
		gtoken.type = TOK_NUMBER;
		gtoken.number = strtol(gtoken.buffer, NULL, 16);
		gtoken.length = i;
	}
	else if(c == '[')
		gtoken.type = TOK_OPENLIST;
	else if(c == ']')
		gtoken.type = TOK_CLOSELIST;
	else if(c == '(')
		gtoken.type = TOK_OPENPAIR;
	else if(c == ')')
		gtoken.type = TOK_CLOSEPAIR;
	else if(c == '@')
	{
		i = 0;
		c = fgetc(in);
		while(isalnum(c) || c == '_' || c == '.')
		{
			if(i < MAX_TOKEN_LEN - 1)
				gtoken.buffer[i++] = c;
			else
				gfatal_error("call exceeded maximum token length (%ld)\n", MAX_TOKEN_LEN);
			c = fgetc(in);
		}
		ungetc(c, in);
		gtoken.buffer[i] = '\0';
		gtoken.type = TOK_CALL;
		gtoken.length = i;
	}
	else if(isdigit(c) || c == '.' || c == '-')
	{
		i = 0;
		while(isdigit(c) || c == '.' || c == '-')
		{
			if(i < MAX_TOKEN_LEN - 1)
				gtoken.buffer[i++] = c;
			else
				gfatal_error("number exceeded maximum token length (%ld)\n", MAX_TOKEN_LEN);
			c = fgetc(in); 
		}
		ungetc(c, in);
		gtoken.buffer[i] = '\0';
		gtoken.type = TOK_NUMBER;
		
		gtoken.length = i;
		gtoken.number = (float)strtod(gtoken.buffer, NULL); 
	}
	else if(c == '\'')
	{
		gtoken.buffer[0] = '\'';
		c = fgetc(in);
		if(c == '\\')
		{
			c = fgetc(in);
			c = escape_seq(c);
		}
		gtoken.number = (float)c;
		gtoken.buffer[1] = c;
		gtoken.type = TOK_NUMBER;
		gtoken.buffer[2] = '\'';
		c = fgetc(in);
	}
	else if(c == '"')
	{
		c = fgetc(in);
		i = 0;
		while(c != '"')
		{			
			if(c == '\\')
			{
				c = fgetc(in);
				c = escape_seq(c);
			}
			if(i < MAX_TOKEN_LEN - 1) 
				gtoken.buffer[i++] = c;
			else
				gfatal_error("string exceeded maximum token length (%ld)\n", MAX_TOKEN_LEN);
			c = fgetc(in);
		}
		gtoken.buffer[i] = '\0';
		gtoken.type = TOK_STRING;
		gtoken.length = i;
	}
	else if(c == '{')
		gtoken.type = TOK_OPENBLOCK;
	else if(c == '}')
		gtoken.type = TOK_CLOSEBLOCK;
	else if(c == EOF)
		gtoken.type = TOK_EOF;
	else if(c == '#')
	{
		c = fgetc(in);
		while(c != '\n' && c != EOF)
			c = fgetc(in);
		if(c == EOF)
			gtoken.type = TOK_EOF;
		else 
		{
			ungetc(c, in);
			gread_token(in);
		}
	}
	
	gerrlineno = gtoken.lineno;
}

void gpeek_token()
{
	FILE* in = ginput_stream;
	gread_token(in);
	long i = gtoken.length;
	if(gtoken.type == TOK_EOF)
		return;
	if(gtoken.type == TOK_STRING)
		ungetc('"', in);
	if(gtoken.type == TOK_OPENBLOCK)
		ungetc('{', in);
	if(gtoken.type == TOK_CLOSEBLOCK)
		ungetc('}', in);
	while(i)
	{
		ungetc(gtoken.buffer[gtoken.length - i], in);
		--i;
	}
	if(gtoken.type == TOK_STRING)
		ungetc('"', in);
	ungetc(' ', in);
}

