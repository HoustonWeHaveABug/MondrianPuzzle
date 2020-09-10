MONDRIAN_PG_C_FLAGS=-pg -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings
MONDRIAN_PG_OBJS=mondrian_pg.o mp_utils_pg.o

mondrian_pg: ${MONDRIAN_PG_OBJS}
	gcc -pg -o mondrian_pg ${MONDRIAN_PG_OBJS}

mondrian_pg.o: mondrian.c mondrian_pg.make
	gcc -c ${MONDRIAN_PG_C_FLAGS} -o mondrian_pg.o mondrian.c

mp_utils_pg.o: mp_utils.h mp_utils.c mondrian_pg.make
	gcc -c ${MONDRIAN_PG_C_FLAGS} -o mp_utils_pg.o mp_utils.c

clean:
	rm -f mondrian_pg ${MONDRIAN_PG_OBJS}
