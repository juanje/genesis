INCLUDES = $(GLIB_CFLAGS)     \
           $(GTK_CFLAGS)      \
           $(LIBXML_CFLAGS)   \
           $(LIBWNCK_CFLAGS)

AM_LDFLAGS = -module -avoid-version

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgenesis.pc

pkgincludedir = $(includedir)/genesis/genesis
pkginclude_HEADERS = genesis.h            \
                     genesis-app-entry.h  \
                     genesis-controller.h

genesis-marshalers.c:
	$(LIBTOOL) --mode=execute $(GLIB_GENMARSHAL) genesis-marshalers.list --body --header >> genesis-marshalers.c

genesis-marshalers.h:
	$(LIBTOOL) --mode=execute $(GLIB_GENMARSHAL) genesis-marshalers.list --header >> genesis-marshalers.h

BUILT_SOURCES = genesis-marshalers.c \
                genesis-marshalers.h

genesis_LTLIBRARIES = libgenesis.la

noinst_HEADERS = genesis-marshalers.h \
                 genesis-common.h     \
                 genesis-fs-monitor.h \
                 genesis-utils.h

libgenesis_la_LIBADD = $(GLIB_LIBS)     \
                       $(GTK_LIBS)      \
                       $(LIBXML_LIBS)   \
                       $(LIBWNCK_LIBS)

libgenesis_la_SOURCES = $(BUILT_SOURCES)     \
                        genesis-utils.c      \
                        genesis-fs-monitor.c \
                        genesis-app-entry.c  \
                        genesis-controller.c

CLEANFILES = $(BUILT_SOURCES) *~ *.la *.o *.so *.lo *.loT *.pc
