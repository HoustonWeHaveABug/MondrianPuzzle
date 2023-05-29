MONDRIAN_VIEW_C_FLAGS=-c -O2 -std=c89 -Wpedantic -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch-default -Wswitch-enum -Wbad-function-cast -Wstrict-overflow=5 -Wundef -Wlogical-op -Wfloat-equal -Wold-style-definition

mondrian_view: mondrian_view.o
	gcc -o mondrian_view mondrian_view.o

mondrian_view.o: mondrian_view.c mondrian_view.make
	gcc ${MONDRIAN_VIEW_C_FLAGS} -o mondrian_view.o mondrian_view.c

clean:
	rm -f mondrian_view mondrian_view.o
