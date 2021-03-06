INCLUDES = $(GLIB_CFLAGS)     \
           $(GTK_CFLAGS)      \
           $(LIBXML_CFLAGS)   \
		   $(DBUS_GLIB_CFLAGS) \
           $(LIBWNCK_CFLAGS)

AM_LDFLAGS = -module -avoid-version

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = genesis.pc

pkgincludedir = $(includedir)/genesis
pkginclude_HEADERS = genesis.h            \
                     genesis-app-entry.h  \
                     genesis-controller.h

genesis-marshalers.c:
	$(LIBTOOL) --mode=execute $(GLIB_GENMARSHAL) genesis-marshalers.list --body --header >> genesis-marshalers.c

genesis-marshalers.h:
	$(LIBTOOL) --mode=execute $(GLIB_GENMARSHAL) genesis-marshalers.list --header >> genesis-marshalers.h

genesis-daemon-dbus-glue.h: genesis-dbus.xml
	$(DBUS_BINDING_TOOL) --prefix=genesis_dbusobj --mode=glib-server --output=$@ $<

genesis-dbus-proxy-glue.h: genesis-dbus.xml
	$(DBUS_BINDING_TOOL) --prefix=genesis_dbusobj --mode=glib-client --output=$@ $<


BUILT_SOURCES = genesis-marshalers.c \
                genesis-marshalers.h \
				genesis-daemon-dbus-glue.h \
				genesis-dbus-proxy-glue.h

genesis_LTLIBRARIES = libgenesis.la

noinst_HEADERS = genesis-marshalers.h \
                 genesis-common.h     \
                 genesis-fs-monitor.h \
                 genesis-utils.h

libgenesis_la_LIBADD = $(GLIB_LIBS)     \
                       $(GTK_LIBS)      \
                       $(LIBXML_LIBS)   \
                       $(LIBWNCK_LIBS)	\
					   $(DBUS_GLIB_LIBS)

libgenesis_la_SOURCES = $(BUILT_SOURCES)     \
                        genesis-utils.c      \
                        genesis-fs-monitor.c \
                        genesis-app-entry.c  \
                        genesis-controller.c \
						genesis-proxy.c

bin_PROGRAMS = genesisd
genesisd_CPPFLAGS = $(INCLUDES)

genesisd_LDFLAGS = $(GLIB_LIBS)     \
                   $(GTK_LIBS)      \
                   $(LIBXML_LIBS)   \
                   $(LIBWNCK_LIBS)  \
				   $(DBUS_GLIB_LIBS)

genesisd_LDADD = $(top_builddir)/src/libgenesis.la

genesisd_SOURCES = genesis-daemon.c genesis-daemon-dbus.c


CLEANFILES = $(BUILT_SOURCES) *~ *.la *.o *.so *.lo *.loT *.pc
