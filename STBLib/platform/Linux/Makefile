#
# OMRON Falcon
# @description	STB
# @user	
#

#
# source directories
srcdir01 = ../../src/src

#
# include directories
incdir  =
incdir += -I../../src/src
incdir += -I../../src/src/STB
incdir += -I../../src/src/STB_FaceRecognition
incdir += -I../../src/src/STB_Property
incdir += -I../../src/src/STB_Tracker
incdir += -I../../src/src/include
incdir += -I.
incdir += -I..

# object directory
objdir_rel = ./release
objdir_deb = ./debug

# library directory
libdir = ../../bin/Linux

#
# Library name
lib = STB

# Library version
#libver = .1.1.0
libver =

# ライブラリ名
static_okaolib = $(libdir)/$(lib).a
static_okaolib_deb = $(libdir)/$(lib)_d.a

# シェアードライブラリ名
shared_okaolib = $(libdir)/lib$(lib).so$(libver)
shared_okaolib_deb = $(libdir)/lib$(lib)_d.so$(libver)

# C flags
#
CFLAGS =  -c -O2 -fPIC -Wall -W -DLINUX_CC -DUNICODE
CFLAGS += $(incdir)

CFLAGS_DEBUG = -g -c -O2 -fPIC -Wall -W -DLINUX_CC -DUNICODE -DHVC_DEBUG
CFLAGS_DEBUG += $(incdir)

LDFLAGS = -shared -Wl,-soname,lib$(lib).so -L$(libdir) -lc
#LDFLAGS += -lhasp_linux
LDFLAGS_DEBUG = -shared -Wl,-soname,lib$(lib)_d.so -L$(libdir) -lc

#
# compilers
CC=gcc
LD=gcc
AR=ar
RM=rm

# Objects
objects_rel = $(objdir_rel)/Interface.o \
              $(objdir_rel)/STBAPI.o \
              $(objdir_rel)/STBFaceInfo.o \
              $(objdir_rel)/STBMakeResult.o \
              $(objdir_rel)/STBTracking.o \
              $(objdir_rel)/STBValidValue.o \
              $(objdir_rel)/FrInterface.o \
              $(objdir_rel)/SdkSTBFr.o \
              $(objdir_rel)/STBFrAPI.o \
              $(objdir_rel)/STBFrValidValue.o \
              $(objdir_rel)/PeInterface.o \
              $(objdir_rel)/SdkSTBPe.o \
              $(objdir_rel)/STBPeAPI.o \
              $(objdir_rel)/STBPeValidValue.o \
              $(objdir_rel)/SdkSTBTr.o \
              $(objdir_rel)/STBTrAPI.o \
              $(objdir_rel)/STBTrValidValue.o \
              $(objdir_rel)/TrInterface.o

objects_deb = $(objdir_deb)/Interface.o \
              $(objdir_deb)/STBAPI.o \
              $(objdir_deb)/STBFaceInfo.o \
              $(objdir_deb)/STBMakeResult.o \
              $(objdir_deb)/STBTracking.o \
              $(objdir_deb)/STBValidValue.o \
              $(objdir_deb)/FrInterface.o \
              $(objdir_deb)/SdkSTBFr.o \
              $(objdir_deb)/STBFrAPI.o \
              $(objdir_deb)/STBFrValidValue.o \
              $(objdir_deb)/PeInterface.o \
              $(objdir_deb)/SdkSTBPe.o \
              $(objdir_deb)/STBPeAPI.o \
              $(objdir_deb)/STBPeValidValue.o \
              $(objdir_deb)/SdkSTBTr.o \
              $(objdir_deb)/STBTrAPI.o \
              $(objdir_deb)/STBTrValidValue.o \
              $(objdir_deb)/TrInterface.o

#
all: makedir $(static_okaolib) $(shared_okaolib) $(static_okaolib_deb) $(shared_okaolib_deb)

# OKAO Library
$(static_okaolib) : $(objects_rel)
	$(AR) -r $@ $(objects_rel)

$(shared_okaolib) : $(objects_rel)
	$(LD) $(LDFLAGS) -o $@ $^

$(static_okaolib_deb) : $(objects_deb)
	$(AR) -r $@ $(objects_deb)

$(shared_okaolib_deb) : $(objects_deb)
	$(LD) $(LDFLAGS_DEBUG) -o $@ $^

# Object
#
# Release
$(objdir_rel)/Interface.o : $(srcdir01)/STB/Interface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/Interface.o $<
$(objdir_rel)/STBAPI.o : $(srcdir01)/STB/STBAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBAPI.o $<
$(objdir_rel)/STBFaceInfo.o : $(srcdir01)/STB/STBFaceInfo.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBFaceInfo.o $<
$(objdir_rel)/STBMakeResult.o : $(srcdir01)/STB/STBMakeResult.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBMakeResult.o $<
$(objdir_rel)/STBTracking.o : $(srcdir01)/STB/STBTracking.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBTracking.o $<
$(objdir_rel)/STBValidValue.o : $(srcdir01)/STB/STBValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBValidValue.o $<
$(objdir_rel)/FrInterface.o : $(srcdir01)/STB_FaceRecognition/FrInterface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/FrInterface.o $<
$(objdir_rel)/SdkSTBFr.o : $(srcdir01)/STB_FaceRecognition/SdkSTBFr.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/SdkSTBFr.o $<
$(objdir_rel)/STBFrAPI.o : $(srcdir01)/STB_FaceRecognition/STBFrAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBFrAPI.o $<
$(objdir_rel)/STBFrValidValue.o : $(srcdir01)/STB_FaceRecognition/STBFrValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBFrValidValue.o $<
$(objdir_rel)/PeInterface.o : $(srcdir01)/STB_Property/PeInterface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/PeInterface.o $<
$(objdir_rel)/SdkSTBPe.o : $(srcdir01)/STB_Property/SdkSTBPe.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/SdkSTBPe.o $<
$(objdir_rel)/STBPeAPI.o : $(srcdir01)/STB_Property/STBPeAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBPeAPI.o $<
$(objdir_rel)/STBPeValidValue.o : $(srcdir01)/STB_Property/STBPeValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBPeValidValue.o $<
$(objdir_rel)/SdkSTBTr.o : $(srcdir01)/STB_Tracker/SdkSTBTr.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/SdkSTBTr.o $<
$(objdir_rel)/STBTrAPI.o : $(srcdir01)/STB_Tracker/STBTrAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBTrAPI.o $<
$(objdir_rel)/STBTrValidValue.o : $(srcdir01)/STB_Tracker/STBTrValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/STBTrValidValue.o $<
$(objdir_rel)/TrInterface.o : $(srcdir01)/STB_Tracker/TrInterface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_rel)/TrInterface.o $<

# Debug
$(objdir_deb)/Interface.o : $(srcdir01)/STB/Interface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/Interface.o $<
$(objdir_deb)/STBAPI.o : $(srcdir01)/STB/STBAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBAPI.o $<
$(objdir_deb)/STBFaceInfo.o : $(srcdir01)/STB/STBFaceInfo.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBFaceInfo.o $<
$(objdir_deb)/STBMakeResult.o : $(srcdir01)/STB/STBMakeResult.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBMakeResult.o $<
$(objdir_deb)/STBTracking.o : $(srcdir01)/STB/STBTracking.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBTracking.o $<
$(objdir_deb)/STBValidValue.o : $(srcdir01)/STB/STBValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBValidValue.o $<
$(objdir_deb)/FrInterface.o : $(srcdir01)/STB_FaceRecognition/FrInterface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/FrInterface.o $<
$(objdir_deb)/SdkSTBFr.o : $(srcdir01)/STB_FaceRecognition/SdkSTBFr.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/SdkSTBFr.o $<
$(objdir_deb)/STBFrAPI.o : $(srcdir01)/STB_FaceRecognition/STBFrAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBFrAPI.o $<
$(objdir_deb)/STBFrValidValue.o : $(srcdir01)/STB_FaceRecognition/STBFrValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBFrValidValue.o $<
$(objdir_deb)/PeInterface.o : $(srcdir01)/STB_Property/PeInterface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/PeInterface.o $<
$(objdir_deb)/SdkSTBPe.o : $(srcdir01)/STB_Property/SdkSTBPe.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/SdkSTBPe.o $<
$(objdir_deb)/STBPeAPI.o : $(srcdir01)/STB_Property/STBPeAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBPeAPI.o $<
$(objdir_deb)/STBPeValidValue.o : $(srcdir01)/STB_Property/STBPeValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBPeValidValue.o $<
$(objdir_deb)/SdkSTBTr.o : $(srcdir01)/STB_Tracker/SdkSTBTr.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/SdkSTBTr.o $<
$(objdir_deb)/STBTrAPI.o : $(srcdir01)/STB_Tracker/STBTrAPI.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBTrAPI.o $<
$(objdir_deb)/STBTrValidValue.o : $(srcdir01)/STB_Tracker/STBTrValidValue.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/STBTrValidValue.o $<
$(objdir_deb)/TrInterface.o : $(srcdir01)/STB_Tracker/TrInterface.c
	$(CC) $(CFLAGS_DEBUG) -o $(objdir_deb)/TrInterface.o $<

allclean : clean libclean

clean : 
	$(RM) -f $(objects_rel) $(objects_deb)

libclean : 
	$(RM) -f $(static_okaolib) $(shared_okaolib) $(static_okaolib_deb) $(shared_okaolib_deb)

.PHONY : makedir
makedir : 
	mkdir -p $(objdir_rel)
	mkdir -p $(objdir_deb)
	mkdir -p $(libdir)

