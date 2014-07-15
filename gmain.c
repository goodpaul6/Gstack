/* -- gmain.c -- a stack based programming language */
#include "gstack.h"

int main(int argc, char* argv[])
{
	if(argc >= 2)
	{
		FILE* file = fopen(argv[1], "r");
		if(!file) gfatal_error("invalid input file! (%s)\n", argv[1]);
		gstate_t* state = gmake_state();
		gfile_load(state, file);
		gload_stdlib(state);
		gexecute_program(state);
		gfile_unload(state);
		gdestroy_state(state);
		fclose(file);
	}
	else 
		gfatal_error("invalid amount of command line arguments (must be >= 2)\n");
	return 0;
}
