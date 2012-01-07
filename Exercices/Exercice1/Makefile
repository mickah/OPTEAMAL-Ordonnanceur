ROOTDIR= ./
LIBNAME=
LIBFILE=

CC      = gcc
CFLAGS  = -Wall -ansi -pedantic -g -m32
LIBDIR  = 
INCDIR  = -I$(ROOTDIR)
LIBS    = 

###------------------------------
### Main targets 
###------------------------------------------------------------
BINARIES= main

all: $(BINARIES)

###------------------------------
### Main rules 
###------------------------------------------------------------

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(INCDIR)

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)



main: main.o $(LIBFILE)


###------------------------------
### Misc.
###------------------------------------------------------------
.PHONY: clean
clean:
	$(RM) *.o $(BINARIES)

