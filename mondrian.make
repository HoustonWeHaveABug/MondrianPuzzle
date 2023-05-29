MONDRIAN_C_FLAGS=-c -O2 -std=c89 -Wpedantic -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch-default -Wswitch-enum -Wbad-function-cast -Wstrict-overflow=5 -Wundef -Wlogical-op -Wfloat-equal -Wold-style-definition
MONDRIAN_OBJS=mondrian.o mp_utils.o

mondrian: ${MONDRIAN_OBJS}
	gcc -o mondrian ${MONDRIAN_OBJS}

mondrian.o: mondrian.c mondrian.make
	gcc ${MONDRIAN_C_FLAGS} -o mondrian.o mondrian.c

mp_utils.o: mp_utils.h mp_utils.c mondrian.make
	gcc ${MONDRIAN_C_FLAGS} -o mp_utils.o mp_utils.c

clean:
	rm -f mondrian ${MONDRIAN_OBJS}
