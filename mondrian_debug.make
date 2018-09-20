MONDRIAN_DEBUG_C_FLAGS=-g -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

mondrian_debug: mondrian_debug.o
	gcc -g -o mondrian_debug mondrian_debug.o

mondrian_debug.o: mondrian.c mondrian_debug.make
	gcc -c ${MONDRIAN_DEBUG_C_FLAGS} -o mondrian_debug.o mondrian.c

clean:
	rm -f mondrian_debug mondrian_debug.o
