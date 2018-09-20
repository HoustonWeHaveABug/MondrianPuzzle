MONDRIAN_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

mondrian: mondrian.o
	gcc -o mondrian mondrian.o

mondrian.o: mondrian.c mondrian.make
	gcc -c ${MONDRIAN_C_FLAGS} -o mondrian.o mondrian.c

clean:
	rm -f mondrian mondrian.o
