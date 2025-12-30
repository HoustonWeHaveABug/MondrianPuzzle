MONDRIAN_C_FLAGS=-c -fPIE -fsanitize=bounds -fsanitize-undefined-trap-on-error -fstack-clash-protection -fstack-protector-strong -O2 -std=c89 -Waggregate-return -Wall -Walloca -Warith-conversion -Warray-bounds=2 -Wbad-function-cast -Wcast-align=strict -Wcast-qual -Wconversion -Wduplicated-branches -Wduplicated-cond -Werror -Wextra -Wfloat-equal -Wformat=2 -Wformat-overflow=2 -Wformat-security -Wformat-signedness -Wformat-truncation=2 -Wimplicit-fallthrough=3 -Winline -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro -Wl,-z,separate-code -Wlogical-op -Wlong-long -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wnull-dereference -Wold-style-definition -Wpedantic -Wpointer-arith -Wredundant-decls -Wshadow -Wshift-overflow=2 -Wstack-protector -Wstack-usage=1000000 -Wstrict-overflow=4 -Wstrict-prototypes -Wstringop-overflow=4 -Wswitch-default -Wswitch-enum -Wtraditional-conversion -Wtrampolines -Wundef -Wvla -Wwrite-strings
MONDRIAN_OBJS=mondrian.o

mondrian: ${MONDRIAN_OBJS}
	gcc -o mondrian ${MONDRIAN_OBJS}

mondrian.o: mondrian.c mondrian.make
	gcc ${MONDRIAN_C_FLAGS} -o mondrian.o mondrian.c

clean:
	rm -f mondrian ${MONDRIAN_OBJS}
