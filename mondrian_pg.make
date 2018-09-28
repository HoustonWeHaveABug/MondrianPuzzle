MONDRIAN_PG_C_FLAGS=-pg -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

mondrian_pg: mondrian_pg.o
	gcc -pg -o mondrian_pg mondrian_pg.o

mondrian_pg.o: mondrian.c mondrian_pg.make
	gcc -c ${MONDRIAN_PG_C_FLAGS} -o mondrian_pg.o mondrian.c

clean:
	rm -f mondrian_pg mondrian_pg.o
