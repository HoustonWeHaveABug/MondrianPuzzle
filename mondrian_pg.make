MONDRIAN_PG_C_FLAGS=-c -pg -std=c89 -Wpedantic -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch-default -Wswitch-enum -Wbad-function-cast -Wstrict-overflow=5 -Wundef -Wlogical-op -Wfloat-equal -Wold-style-definition
MONDRIAN_PG_OBJS=mondrian_pg.o mp_utils_pg.o

mondrian_pg: ${MONDRIAN_PG_OBJS}
	gcc -pg -o mondrian_pg ${MONDRIAN_PG_OBJS}

mondrian_pg.o: mondrian.c mondrian_pg.make
	gcc ${MONDRIAN_PG_C_FLAGS} -o mondrian_pg.o mondrian.c

mp_utils_pg.o: mp_utils.h mp_utils.c mondrian_pg.make
	gcc ${MONDRIAN_PG_C_FLAGS} -o mp_utils_pg.o mp_utils.c

clean:
	rm -f mondrian_pg ${MONDRIAN_PG_OBJS}
