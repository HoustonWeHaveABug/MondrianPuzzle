MONDRIAN_VIEW_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

mondrian_view: mondrian_view.o
	gcc -o mondrian_view mondrian_view.o

mondrian_view.o: mondrian_view.c mondrian_view.make
	gcc ${MONDRIAN_VIEW_C_FLAGS} -o mondrian_view.o mondrian_view.c

clean:
	rm -f mondrian_view mondrian_view.o
