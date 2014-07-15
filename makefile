FILES = gstack.h glexer.c gstate.c gobject.c gcollector.c gparser.c gexecute.c gutils.c gstdlib.c gmain.c
OUT = gstack

build: $(FILES)	
	gcc -Wall -o $(OUT) $(FILES)

clean:
	rm -f $(OUT)

rebuild: clean build
