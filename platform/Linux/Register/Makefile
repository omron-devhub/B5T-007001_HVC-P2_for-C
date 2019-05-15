#
# OMRON HVC-P2
# @description	Register
# @user	
#

#
# include directories
incdir  =
incdir += -I../../../src
incdir += -I../../../src/bmp
incdir += -I../../../src/HVCApi
incdir += -I../../../src/Sample
incdir += -I../../../src/uart
incdir += -I.

# object directory
objdir_release = ./release
objdir_debug = ./debug

# module directory
exedir = ../../../bin/Linux

GOAL     = Register
# ÉÇÉWÉÖÅ[Éãñº
exe_release = $(exedir)/$(GOAL)
exe_debug = $(exedir)/$(GOAL)_d

LIBS += 

# C flags
#
CFLAGS_RELEASE =  -c -Os -fPIC -Wall -W -DLINUX_CC -DUNICODE
CFLAGS_RELEASE += $(incdir)

CFLAGS_DEBUG = -g -c -Os -fPIC -Wall -W -DLINUX_CC -DUNICODE -DHVC_DEBUG
CFLAGS_DEBUG += $(incdir)

LDFLAGS_RELEASE = -shared -Wl,-soname=lib$(GOAL).so -L$(LIBS) -lc
LDFLAGS_DEBUG = -shared -Wl,-soname=lib$(GOAL)_d.so -L$(LIBS) -lc

EXFLAGS = -lpthread -lrt -lm

#
# compilers
CC=gcc
LD=gcc
AR=ar
RM=rm

# Objects
objects_release = \
              $(objdir_release)/register_main.o\
              $(objdir_release)/Album.o\
              $(objdir_release)/HVCApi.o\
              $(objdir_release)/bitmap_linux.o\
              $(objdir_release)/uart_linux.o\

objects_debug = \
              $(objdir_debug)/register_main.o\
              $(objdir_debug)/Album.o\
              $(objdir_debug)/HVCApi.o\
              $(objdir_debug)/bitmap_linux.o\
              $(objdir_debug)/uart_linux.o\

all: makedir $(exe_release) $(exe_debug)

# Module
$(exe_release) : $(objects_release)
	$(LD) -o $@ $(objects_release) $(EXFLAGS)

$(exe_debug) : $(objects_debug)
	$(LD) -o $@ $(objects_debug) $(EXFLAGS)


# Object
#
# Release
$(objdir_release)/register_main.o : ../../../src/FR_Sample/register_main.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/register_main.o $<
$(objdir_release)/HVCApi.o : ../../../src/HVCApi/HVCApi.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/HVCApi.o $<
$(objdir_release)/Album.o : ../../../src/Album/Album.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/Album.o $<
$(objdir_release)/bitmap_linux.o : ../../../src/bmp/bitmap_linux.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/bitmap_linux.o $<
$(objdir_release)/uart_linux.o : ../../../src/uart/uart_linux.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/uart_linux.o $<


# Debug
$(objdir_debug)/register_main.o : ../../../src/FR_Sample/register_main.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/register_main.o $<
$(objdir_debug)/HVCApi.o : ../../../src/HVCApi/HVCApi.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/HVCApi.o $<
$(objdir_debug)/Album.o : ../../../src/Album/Album.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/Album.o $<
$(objdir_debug)/bitmap_linux.o : ../../../src/bmp/bitmap_linux.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/bitmap_linux.o $<
$(objdir_debug)/uart_linux.o : ../../../src/uart/uart_linux.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/uart_linux.o $<


#

.PHONY: clean
clean:
	$(RM) -f $(objects_release) $(objects_debug) 

.PHONY : makedir
makedir:
	mkdir -p $(objdir_release)
	mkdir -p $(objdir_debug)
	mkdir -p $(exedir)

