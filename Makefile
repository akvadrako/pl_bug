
all: gl/*.h wayeasy/*.h util/*.h
	gcc -o pl_bug -O0 -ggdb -Wall -I. -Iproto -O0 -ggdb -Wall \
	    -lwayland-client -lwayland-egl -lwayland-cursor -lEGL -lGLESv2 -lm -lplacebo \
	    *.c */*.c
