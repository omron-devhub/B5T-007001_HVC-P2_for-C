#
# OMRON HVC-P2
# @description	Sample
# @user	
#

#
# include directories
incdir  =
incdir += -I../../../platform/Sample
incdir += -I../../../src
incdir += -I../../../src/bmp
incdir += -I../../../src/HVCApi
incdir += -I../../../src/Sample
incdir += -I../../../src/uart
incdir += -I../../../src/STBApi
incdir += -I../../../import/include
incdir += -I.

# object directory
objdir_release = ./release
objdir_debug = ./debug

# library directories
libdir  =
libdir +=../../../import/lib


# module directory
exedir = ../../../bin/Linux

GOAL     = Sample
# モジュール名
exe_release = $(exedir)/$(GOAL)
exe_debug = $(exedir)/$(GOAL)_d


#ライブラリ名
LIBS += STB

# C flags
#
CFLAGS_RELEASE =  -c -Os -fPIC -Wall -W -DLINUX_CC -DUNICODE
CFLAGS_RELEASE += $(incdir)

CFLAGS_DEBUG = -g -c -Os -fPIC -Wall -W -DLINUX_CC -DUNICODE -DHVC_DEBUG
CFLAGS_DEBUG += $(incdir)

EXFLAGS = -L$(libdir) -lpthread -lrt -lSTB -lm

#
# compilers
CC=gcc
LD=gcc
AR=ar
RM=rm

# Objects
objects_release = \
              $(objdir_release)/main.o\
              $(objdir_release)/HVCApi.o\
              $(objdir_release)/bitmap_linux.o\
              $(objdir_release)/uart_linux.o\
              $(objdir_release)/STBWrap.o\

objects_debug = \
              $(objdir_debug)/main.o\
              $(objdir_debug)/HVCApi.o\
              $(objdir_debug)/bitmap_linux.o\
              $(objdir_debug)/uart_linux.o\
              $(objdir_debug)/STBWrap.o\

all: makedir $(exe_release) $(exe_debug)

# Module
$(exe_release) : $(objects_release)
	$(LD) -o $@ $(objects_release) $(EXFLAGS)

$(exe_debug) : $(objects_debug)
	$(LD) -o $@ $(objects_debug) $(EXFLAGS)


# Object
#
# Release
$(objdir_release)/main.o : ../../../src/Sample/main.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/main.o $<
$(objdir_release)/HVCApi.o : ../../../src/HVCApi/HVCApi.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/HVCApi.o $<
$(objdir_release)/bitmap_linux.o : ../../../src/bmp/bitmap_linux.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/bitmap_linux.o $<
$(objdir_release)/uart_linux.o : ../../../src/uart/uart_linux.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/uart_linux.o $<
$(objdir_release)/STBWrap.o : ../../../src/STBApi/STBWrap.c
	$(CC) $(CFLAGS_RELEASE) -o $(objdir_release)/STBWrap.o $<


# Debug
$(objdir_debug)/main.o : ../../../src/Sample/main.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/main.o $<
$(objdir_debug)/HVCApi.o : ../../../src/HVCApi/HVCApi.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/HVCApi.o $<
$(objdir_debug)/bitmap_linux.o : ../../../src/bmp/bitmap_linux.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/bitmap_linux.o $<
$(objdir_debug)/uart_linux.o : ../../../src/uart/uart_linux.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/uart_linux.o $<
$(objdir_debug)/STBWrap.o : ../../../src/STBApi/STBWrap.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_debug)/STBWrap.o $<


#

.PHONY: clean
clean:
	$(RM) -f $(objects_release) $(objects_debug) 

.PHONY : makedir
makedir:
	mkdir -p $(objdir_release)
	mkdir -p $(objdir_debug)
	mkdir -p $(exedir)

