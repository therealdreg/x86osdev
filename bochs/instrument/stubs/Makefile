# Copyright (C) 2001  The Bochs Project
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA



.SUFFIXES: .cc

srcdir = .


SHELL = /bin/sh



CC = cl
CFLAGS = /nologo /MT /W3 /DNDEBUG /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS /O2 /Gr /EHsc
CXX = cl
CXXFLAGS = /nologo /MT /W3 /DNDEBUG /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS /O2 /Gr /EHsc

LDFLAGS = 
LIBS = 
RANLIB = echo


# ===========================================================
# end of configurable options
# ===========================================================


BX_OBJS = \
  instrument.o

BX_INCLUDES =

BX_INCDIRS = -I../.. -I$(srcdir)/../.. -I. -I$(srcdir)/.

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(BX_INCDIRS) /Tp$< /Fo$@


.c.o:
	$(CC) -c $(CFLAGS) $(BX_INCDIRS) /Tc$< /Fo$@



libinstrument.a: $(BX_OBJS)
	-del libinstrument.a
	lib /nologo /subsystem:console /verbose /out:$@ $(BX_OBJS)
	$(RANLIB) libinstrument.a

$(BX_OBJS): $(BX_INCLUDES)


clean:
	-del *.o
	-del *.a

dist-clean: clean
	-del Makefile
