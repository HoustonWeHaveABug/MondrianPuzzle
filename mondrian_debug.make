MONDRIAN_DEBUG_C_FLAGS=-g -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings
MONDRIAN_DEBUG_OBJS=mondrian_debug.o mp_utils_debug.o

mondrian_debug: ${MONDRIAN_DEBUG_OBJS}
	gcc -g -o mondrian_debug ${MONDRIAN_DEBUG_OBJS}

mondrian_debug.o: mondrian.c mondrian_debug.make
	gcc -c ${MONDRIAN_DEBUG_C_FLAGS} -o mondrian_debug.o mondrian.c

mp_utils_debug.o: mp_utils.h mp_utils.c mondrian_debug.make
	gcc -c ${MONDRIAN_DEBUG_C_FLAGS} -o mp_utils_debug.o mp_utils.c

clean:
	rm -f mondrian_debug ${MONDRIAN_DEBUG_OBJS}
