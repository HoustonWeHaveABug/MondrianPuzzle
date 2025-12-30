MONDRIAN_DEBUG_C_FLAGS=-c -fPIE -fsanitize=bounds -fsanitize-undefined-trap-on-error -fstack-clash-protection -fstack-protector-strong -g -std=c89 -Waggregate-return -Wall -Walloca -Warith-conversion -Warray-bounds=2 -Wbad-function-cast -Wcast-align=strict -Wcast-qual -Wconversion -Wduplicated-branches -Wduplicated-cond -Werror -Wextra -Wfloat-equal -Wformat=2 -Wformat-overflow=2 -Wformat-security -Wformat-signedness -Wformat-truncation=2 -Wimplicit-fallthrough=3 -Winline -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro -Wl,-z,separate-code -Wlogical-op -Wlong-long -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wnull-dereference -Wold-style-definition -Wpedantic -Wpointer-arith -Wredundant-decls -Wshadow -Wshift-overflow=2 -Wstack-protector -Wstack-usage=1000000 -Wstrict-overflow=4 -Wstrict-prototypes -Wstringop-overflow=4 -Wswitch-default -Wswitch-enum -Wtraditional-conversion -Wtrampolines -Wundef -Wvla -Wwrite-strings
MONDRIAN_DEBUG_OBJS=mondrian_debug.o

mondrian_debug: ${MONDRIAN_DEBUG_OBJS}
	gcc -g -o mondrian_debug ${MONDRIAN_DEBUG_OBJS}

mondrian_debug.o: mondrian.c mondrian_debug.make
	gcc ${MONDRIAN_DEBUG_C_FLAGS} -o mondrian_debug.o mondrian.c

clean:
	rm -f mondrian_debug ${MONDRIAN_DEBUG_OBJS}
