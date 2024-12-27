CC = gcc

CFLAGS = -lglut -lGL -lGLU -lm

SRCS = game.c

all: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o Viewer

clean:
	rm -f Viewer
