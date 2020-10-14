MONDRIAN_VIEW_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings
MONDRIAN_VIEW_OBJS=mondrian_view.o

mondrian_view: ${MONDRIAN_VIEW_OBJS}
	gcc -o mondrian_view ${MONDRIAN_VIEW_OBJS}

mondrian_view.o: mondrian_view.c mondrian_view.make
	gcc -c ${MONDRIAN_VIEW_C_FLAGS} -o mondrian_view.o mondrian_view.c

clean:
	rm -f mondrian_view ${MONDRIAN_VIEW_OBJS}
