MONDRIAN_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings
MONDRIAN_OBJS=mondrian.o mp_utils.o

mondrian: ${MONDRIAN_OBJS}
	gcc -o mondrian ${MONDRIAN_OBJS}

mondrian.o: mondrian.c mondrian.make
	gcc ${MONDRIAN_C_FLAGS} -o mondrian.o mondrian.c

mp_utils.o: mp_utils.h mp_utils.c mondrian.make
	gcc ${MONDRIAN_C_FLAGS} -o mp_utils.o mp_utils.c

clean:
	rm -f mondrian ${MONDRIAN_OBJS}
