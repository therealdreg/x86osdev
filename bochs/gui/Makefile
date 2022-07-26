# Copyright (C) 2001-2021  The Bochs Project
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

# Makefile for the gui component of bochs


.SUFFIXES: .cc

prefix          = /usr/local
exec_prefix     = $(prefix)
srcdir          = .

bindir          = $(exec_prefix)/bin
libdir          = $(exec_prefix)/lib
datarootdir     = $(prefix)/share
mandir          = $(datarootdir)/man
man1dir         = $(mandir)/man1
man5dir         = $(mandir)/man5
docdir          = $(datarootdir)/doc/bochs
sharedir        = $(datarootdir)/bochs
top_builddir    = ..
top_srcdir      = ..

SHELL = /bin/sh



CXX = cl
CXXFLAGS = $(BX_INCDIRS) /nologo /MT /W3 /DNDEBUG /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS /O2 /Gr /EHsc  
LOCAL_CXXFLAGS = 
LDFLAGS = 
LIBS = 
X_CFLAGS = 
RANLIB = echo
PLUGIN_PATH=$(exec_prefix)/lib
top_builddir    = ..
LIBTOOL=
WIN32_DLL_IMPORT_LIBRARY=../
BX_INCDIRS = -I.. -I$(srcdir)/.. -I../iodev -I$(srcdir)/../iodev -I../instrument/stubs -I$(srcdir)/../instrument/stubs

GUI_OBJS_X11  = x.o
GUI_OBJS_SDL = sdl.o
GUI_OBJS_SDL2 = sdl2.o
GUI_OBJS_WIN32 = win32.o
GUI_OBJS_MACOS = macintosh.o
GUI_OBJS_CARBON = carbon.o
GUI_OBJS_NOGUI = nogui.o
GUI_OBJS_TERM  = term.o
GUI_OBJS_RFB = rfb.o
GUI_OBJS_VNCSRV = vncsrv.o
GUI_OBJS_AMIGAOS = amigaos.o
GUI_OBJS_WX = wx.o
GUI_OBJS_WX_SUPPORT = wxmain.o wxdialog.o

CI_OBJS_WIN32 = win32config.o
CI_OBJS_WIN32_SUPPORT = win32paramdlg.o scrollwin.o

GUI_DLL_TARGETS =  bx_rfb_gui.dll bx_win32_gui.dll bx_nogui_gui.dll
OBJS_THAT_CANNOT_BE_PLUGINS = keymap.o gui.o siminterface.o paramtree.o enh_dbg.o win32_enh_dbg_osdep.o

X_LIBS = 
X_PRE_LIBS = 
GUI_LINK_OPTS_X = $(X_LIBS) $(X_PRE_LIBS)
GUI_LINK_OPTS_SDL = 
GUI_LINK_OPTS_SDL2 = 
GUI_LINK_OPTS_RFB =  ws2_32.lib
GUI_LINK_OPTS_RFB_VCPP = user32.lib  ws2_32.lib
GUI_LINK_OPTS_VNCSRV = 
GUI_LINK_OPTS_AMIGAOS =
GUI_LINK_OPTS_WIN32 = -luser32 -lgdi32 -lcomctl32
GUI_LINK_OPTS_WIN32_VCPP = user32.lib gdi32.lib comctl32.lib
GUI_LINK_OPTS_MACOS =
GUI_LINK_OPTS_CARBON = -framework Carbon
GUI_LINK_OPTS_NOGUI =
GUI_LINK_OPTS_TERM = 
GUI_LINK_OPTS_WX = 

CI_LINK_OPTS_WIN32 = -luser32 -lgdi32 -lcomctl32 -lcomdlg32 -lshell32
CI_LINK_OPTS_WIN32_VCPP = user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib

GUI_PLUGIN_OBJS =  $(GUI_OBJS_RFB) $(GUI_OBJS_WIN32) $(GUI_OBJS_NOGUI)
CI_PLUGIN_OBJS = textconfig.o $(CI_OBJS_WIN32)
OBJS_THAT_CAN_BE_PLUGINS = $(GUI_PLUGIN_OBJS) $(CI_PLUGIN_OBJS)
OBJS_THAT_SUPPORT_OTHER_PLUGINS =  $(CI_OBJS_WIN32_SUPPORT)
NONPLUGIN_OBJS = $(OBJS_THAT_CANNOT_BE_PLUGINS) $(OBJS_THAT_CAN_BE_PLUGINS) $(OBJS_THAT_SUPPORT_OTHER_PLUGINS)

#
# -------- end configurable options --------------------------
#

all: libgui.a

plugins: 

plugins_gcc: $(GUI_PLUGIN_OBJS:%.o=libbx_%_gui.la) $(CI_PLUGIN_OBJS:%.o=libbx_%.la)

plugins_msvc: $(GUI_DLL_TARGETS) bx_textconfig.dll bx_win32config.dll

libgui.a: $(NONPLUGIN_OBJS)
	-del libgui.a
	lib /nologo /subsystem:console /verbose /out:$@ $(NONPLUGIN_OBJS)
	$(RANLIB) libgui.a

# standard compile rule for C++ files
.cc.o:
	$(CXX) /c  $(CXXFLAGS) $(LOCAL_CXXFLAGS) /Tp$< /Fo$@

##### building plugins with libtool
%.lo: %.cc
	$(LIBTOOL) --mode=compile --tag CXX $(CXX) -c $(CXXFLAGS) $(LOCAL_CXXFLAGS) $< -o $@

libbx_%_gui.la: %.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH)

libbx_x_gui.la: x.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_X)

libbx_sdl_gui.la: sdl.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_SDL)

libbx_sdl2_gui.la: sdl2.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_SDL2)

libbx_rfb_gui.la: rfb.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_RFB)

libbx_vncsrv_gui.la: vncsrv.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_VNCSRV)

libbx_amigaos_gui.la: amigaos.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_AMIGAOS)

libbx_win32_gui.la: win32.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_WIN32)

libbx_macos_gui.la: macos.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_MACOS)

libbx_carbon_gui.la: carbon.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_CARBON)

libbx_nogui_gui.la: nogui.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_NOGUI)

libbx_term_gui.la: term.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH) $(GUI_LINK_OPTS_TERM)

libbx_textconfig.la: textconfig.lo
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $< -o $@ -rpath $(PLUGIN_PATH)

# special link rules for plugins that require more than one object file
libbx_wx_gui.la: $(GUI_OBJS_WX:.o=.lo) $(GUI_OBJS_WX_SUPPORT:.o=.lo)
	$(LIBTOOL) --mode=link --tag CXX $(CXX) -module $(GUI_OBJS_WX:.o=.lo) $(GUI_OBJS_WX_SUPPORT:.o=.lo) -o $@ -rpath $(PLUGIN_PATH)  $(GUI_LINK_OPTS_WX)

#### building DLLs for win32 (Cygwin and MinGW/MSYS)
bx_%_gui.dll: %.o
	$(CXX) $(CXXFLAGS) -shared -o $@ $< $(WIN32_DLL_IMPORT_LIBRARY)  $(GUI_LINK_OPTS_WIN32)

bx_wx_gui.dll: $(GUI_OBJS_WX) $(GUI_OBJS_WX_SUPPORT)
	$(CXX) $(CXXFLAGS) -shared -o bx_wx_gui.dll $(GUI_OBJS_WX) $(GUI_OBJS_WX_SUPPORT) $(WIN32_DLL_IMPORT_LIBRARY) `wx-config --libs` -luser32 -lgdi32 -lcomdlg32 -lcomctl32

bx_sdl_gui.dll: $(GUI_OBJS_SDL)
	$(CXX) $(CXXFLAGS) -shared -o bx_sdl_gui.dll $(GUI_OBJS_SDL) $(WIN32_DLL_IMPORT_LIBRARY) $(GUI_LINK_OPTS_SDL)

bx_sdl2_gui.dll: $(GUI_OBJS_SDL2)
	$(CXX) $(CXXFLAGS) -shared -o bx_sdl2_gui.dll $(GUI_OBJS_SDL2) $(WIN32_DLL_IMPORT_LIBRARY) $(GUI_LINK_OPTS_SDL2)

bx_vncsrv_gui.dll: $(GUI_OBJS_VNCSRV)
	$(CXX) $(CXXFLAGS) -shared -o bx_vncsrv.dll $(GUI_OBJS_VNCSRV) $(WIN32_DLL_IMPORT_LIBRARY) $(GUI_LINK_OPTS_VNCSRV)

# special link rules for plugins with Cygwin, MinGW/MSYS and MSVC nmake
bx_nogui_gui.dll: $(GUI_OBJS_NOGUI)
	 $(GUI_OBJS_NOGUI) $(WIN32_DLL_IMPORT_LIBRARY)

bx_rfb_gui.dll: $(GUI_OBJS_RFB)
	 $(GUI_OBJS_RFB) $(WIN32_DLL_IMPORT_LIBRARY) $(GUI_LINK_OPTS_RFB)

bx_win32_gui.dll: $(GUI_OBJS_WIN32)
	 $(GUI_OBJS_WIN32) $(WIN32_DLL_IMPORT_LIBRARY) $(GUI_LINK_OPTS_WIN32)

bx_textconfig.dll: textconfig.o
	 textconfig.o $(WIN32_DLL_IMPORT_LIBRARY)

bx_win32config.dll: $(CI_OBJS_WIN32) $(CI_OBJS_WIN32_SUPPORT)
	 $(CI_OBJS_WIN32) $(CI_OBJS_WIN32_SUPPORT) $(WIN32_DLL_IMPORT_LIBRARY) $(CI_LINK_OPTS_WIN32)

##### end DLL section

clean:
	-del -rf .libs *.la *.a *.lo *.o *.dll *.exp *.lib

dist-clean: clean
	-del Makefile

###########################################
# all other dependencies generated by
#  gcc -MM -I.. -I../iodev -I../instrument/stubs *.cc | \
#     sed -e 's/\.cc/.cc/g'
#  gcc -MM -I.. -I../iodev -I../instrument/stubs *.cc | \
#     sed -e 's/\.cc/.cc/g' -e 's/\.o:/.lo:/g'
#
# This means that every source file is listed twice, once with a .o rule
# and then again with an identical .lo rule.  The .lo rules are used when
# building plugins.
###########################################
amigaos.o: amigaos.cc ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bxversion.h \
 ../param_names.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
carbon.o: carbon.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h ../param_names.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
enh_dbg.o: enh_dbg.cc ../config.h
gtk_enh_dbg_osdep.o: gtk_enh_dbg_osdep.cc ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bx_debug/debug.h \
 ../config.h ../osdep.h siminterface.h enh_dbg.h
gui.o: gui.cc ../iodev/iodev.h ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../param_names.h ../pc_system.h ../bx_debug/debug.h \
 ../config.h ../osdep.h ../memory/memory-bochs.h ../gui/siminterface.h \
 ../gui/gui.h ../iodev/virt_timer.h keymap.h ../gui/bitmaps/floppya.h \
 ../gui/bitmaps/floppyb.h ../gui/bitmaps/mouse.h ../gui/bitmaps/reset.h \
 ../gui/bitmaps/power.h ../gui/bitmaps/snapshot.h ../gui/bitmaps/copy.h \
 ../gui/bitmaps/paste.h ../gui/bitmaps/configbutton.h \
 ../gui/bitmaps/cdromd.h ../gui/bitmaps/userbutton.h \
 ../gui/bitmaps/saverestore.h sdl.h
keymap.o: keymap.cc ../param_names.h ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h gui.h \
 ../gui/siminterface.h keymap.h
macintosh.o: macintosh.cc ../param_names.h ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../iodev/iodev.h \
 ../plugin.h ../extplugin.h ../pc_system.h ../bx_debug/debug.h \
 ../config.h ../osdep.h ../memory/memory-bochs.h ../gui/siminterface.h \
 ../gui/gui.h
nogui.o: nogui.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 gui.h ../gui/siminterface.h ../plugin.h ../extplugin.h ../param_names.h \
 icon_bochs.h
paramtree.o: paramtree.cc ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h siminterface.h \
 paramtree.h
rfb.o: rfb.cc ../param_names.h ../iodev/iodev.h ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h keymap.h \
 icon_bochs.h font/vga.bitmap.h sdl.h rfb.h rfbkeys.h ../bxthread.h
scrollwin.o: scrollwin.cc ../config.h
sdl2.o: sdl2.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h icon_bochs.h \
 sdl.h sdlkeys.h
sdl.o: sdl.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
siminterface.o: siminterface.cc ../param_names.h ../iodev/iodev.h \
 ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h \
 ../iodev/virt_timer.h
term.o: term.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
textconfig.o: textconfig.cc ../config.h ../osdep.h ../bx_debug/debug.h \
 ../param_names.h ../logio.h paramtree.h siminterface.h ../plugin.h \
 ../extplugin.h
vncsrv.o: vncsrv.cc ../param_names.h ../iodev/iodev.h ../bochs.h \
 ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h keymap.h \
 icon_bochs.h font/vga.bitmap.h sdl.h rfb.h rfbkeys.h ../bxthread.h
win32.o: win32.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
win32config.o: win32config.cc win32dialog.h ../config.h ../bochs.h \
 ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bx_debug/debug.h \
 ../osdep.h ../param_names.h gui.h ../gui/siminterface.h win32res.h \
 win32paramdlg.h ../plugin.h ../extplugin.h
win32_enh_dbg_osdep.o: win32_enh_dbg_osdep.cc ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bx_debug/debug.h \
 ../config.h ../osdep.h siminterface.h win32dialog.h enh_dbg.h
win32paramdlg.o: win32paramdlg.cc win32dialog.h ../config.h ../bochs.h \
 ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h siminterface.h \
 win32res.h
wx.o: wx.cc ../config.h ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../param_names.h \
 keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h ../pc_system.h \
 ../bx_debug/debug.h ../osdep.h ../memory/memory-bochs.h \
 ../gui/siminterface.h ../gui/gui.h font/vga.bitmap.h wxmain.h
wxdialog.o: wxdialog.cc ../config.h ../param_names.h ../osdep.h \
 ../gui/paramtree.h ../gui/siminterface.h ../logio.h wxdialog.h wxmain.h
wxmain.o: wxmain.cc ../config.h ../param_names.h ../osdep.h \
 ../gui/paramtree.h ../gui/siminterface.h ../bxversion.h wxdialog.h \
 wxmain.h ../extplugin.h bitmaps/cdromd.xpm bitmaps/copy.xpm \
 bitmaps/floppya.xpm bitmaps/floppyb.xpm bitmaps/paste.xpm \
 bitmaps/power.xpm bitmaps/reset.xpm bitmaps/snapshot.xpm \
 bitmaps/mouse.xpm bitmaps/userbutton.xpm bitmaps/saverestore.xpm
x.o: x.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h enh_dbg.h \
 icon_bochs.h font/vga.bitmap.h
amigaos.lo: amigaos.cc ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bxversion.h \
 ../param_names.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
carbon.lo: carbon.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h ../param_names.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
enh_dbg.lo: enh_dbg.cc ../config.h
gtk_enh_dbg_osdep.lo: gtk_enh_dbg_osdep.cc ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bx_debug/debug.h \
 ../config.h ../osdep.h siminterface.h enh_dbg.h
gui.lo: gui.cc ../iodev/iodev.h ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../param_names.h ../pc_system.h ../bx_debug/debug.h \
 ../config.h ../osdep.h ../memory/memory-bochs.h ../gui/siminterface.h \
 ../gui/gui.h ../iodev/virt_timer.h keymap.h ../gui/bitmaps/floppya.h \
 ../gui/bitmaps/floppyb.h ../gui/bitmaps/mouse.h ../gui/bitmaps/reset.h \
 ../gui/bitmaps/power.h ../gui/bitmaps/snapshot.h ../gui/bitmaps/copy.h \
 ../gui/bitmaps/paste.h ../gui/bitmaps/configbutton.h \
 ../gui/bitmaps/cdromd.h ../gui/bitmaps/userbutton.h \
 ../gui/bitmaps/saverestore.h sdl.h
keymap.lo: keymap.cc ../param_names.h ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h gui.h \
 ../gui/siminterface.h keymap.h
macintosh.lo: macintosh.cc ../param_names.h ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../iodev/iodev.h \
 ../plugin.h ../extplugin.h ../pc_system.h ../bx_debug/debug.h \
 ../config.h ../osdep.h ../memory/memory-bochs.h ../gui/siminterface.h \
 ../gui/gui.h
nogui.lo: nogui.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 gui.h ../gui/siminterface.h ../plugin.h ../extplugin.h ../param_names.h \
 icon_bochs.h
paramtree.lo: paramtree.cc ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h siminterface.h \
 paramtree.h
rfb.lo: rfb.cc ../param_names.h ../iodev/iodev.h ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h keymap.h \
 icon_bochs.h font/vga.bitmap.h sdl.h rfb.h rfbkeys.h ../bxthread.h
scrollwin.lo: scrollwin.cc ../config.h
sdl2.lo: sdl2.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h icon_bochs.h \
 sdl.h sdlkeys.h
sdl.lo: sdl.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
siminterface.lo: siminterface.cc ../param_names.h ../iodev/iodev.h \
 ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h \
 ../iodev/virt_timer.h
term.lo: term.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
textconfig.lo: textconfig.cc ../config.h ../osdep.h ../bx_debug/debug.h \
 ../param_names.h ../logio.h paramtree.h siminterface.h ../plugin.h \
 ../extplugin.h
vncsrv.lo: vncsrv.cc ../param_names.h ../iodev/iodev.h ../bochs.h \
 ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../plugin.h \
 ../extplugin.h ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h keymap.h \
 icon_bochs.h font/vga.bitmap.h sdl.h rfb.h rfbkeys.h ../bxthread.h
win32.lo: win32.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h \
 ../logio.h ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h
win32config.lo: win32config.cc win32dialog.h ../config.h ../bochs.h \
 ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bx_debug/debug.h \
 ../osdep.h ../param_names.h gui.h ../gui/siminterface.h win32res.h \
 win32paramdlg.h ../plugin.h ../extplugin.h
win32_enh_dbg_osdep.lo: win32_enh_dbg_osdep.cc ../bochs.h ../config.h \
 ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../bx_debug/debug.h \
 ../config.h ../osdep.h siminterface.h win32dialog.h enh_dbg.h
win32paramdlg.lo: win32paramdlg.cc win32dialog.h ../config.h ../bochs.h \
 ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h siminterface.h \
 win32res.h
wx.lo: wx.cc ../config.h ../bochs.h ../config.h ../osdep.h \
 ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h ../param_names.h \
 keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h ../pc_system.h \
 ../bx_debug/debug.h ../osdep.h ../memory/memory-bochs.h \
 ../gui/siminterface.h ../gui/gui.h font/vga.bitmap.h wxmain.h
wxdialog.lo: wxdialog.cc ../config.h ../param_names.h ../osdep.h \
 ../gui/paramtree.h ../gui/siminterface.h ../logio.h wxdialog.h wxmain.h
wxmain.lo: wxmain.cc ../config.h ../param_names.h ../osdep.h \
 ../gui/paramtree.h ../gui/siminterface.h ../bxversion.h wxdialog.h \
 wxmain.h ../extplugin.h bitmaps/cdromd.xpm bitmaps/copy.xpm \
 bitmaps/floppya.xpm bitmaps/floppyb.xpm bitmaps/paste.xpm \
 bitmaps/power.xpm bitmaps/reset.xpm bitmaps/snapshot.xpm \
 bitmaps/mouse.xpm bitmaps/userbutton.xpm bitmaps/saverestore.xpm
x.lo: x.cc ../bochs.h ../config.h ../osdep.h ../gui/paramtree.h ../logio.h \
 ../instrument/stubs/instrument.h ../misc/bswap.h \
 ../param_names.h keymap.h ../iodev/iodev.h ../plugin.h ../extplugin.h \
 ../pc_system.h ../bx_debug/debug.h ../config.h ../osdep.h \
 ../memory/memory-bochs.h ../gui/siminterface.h ../gui/gui.h enh_dbg.h \
 icon_bochs.h font/vga.bitmap.h
