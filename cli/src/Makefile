PROGS=glxsimple

all: $(PROGS)

%:%.c
	$(CC) -Wall -Wextra -g -o $@ $^ -lGL -lX11 -lGLEW

clean:
	rm -f *.o $(PROGS)
