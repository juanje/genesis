INCLUDES = $(GLIB_CFLAGS)     \
           $(GTK_CFLAGS)      \
           $(GNOMEVFS_CFLAGS) \
           $(GCONF_CFLAGS)    \
           $(LIBXML_CFLAGS)   \
           $(LIBWNCK_CFLAGS)

AM_LDFLAGS = -module -avoid-version

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libprobot.pc

pkgincludedir = $(includedir)/libprobot/libprobot
pkginclude_HEADERS = probot-utils.h     \
                     probot-app-entry.h \
                     probot-controller.h

probot-marshalers.c:
	$(LIBTOOL) --mode=execute $(GLIB_GENMARSHAL) probot-marshalers.list --body --header >> probot-marshalers.c

probot-marshalers.h:
	$(LIBTOOL) --mode=execute $(GLIB_GENMARSHAL) probot-marshalers.list --header >> probot-marshalers.h

BUILT_SOURCES = probot-marshalers.c \
                probot-marshalers.h

probot_LTLIBRARIES = libprobot.la

noinst_HEADERS = probot-marshalers.h

libprobot_la_LIBADD = $(GLIB_LIBS)     \
                      $(GTK_LIBS)      \
                      $(GNOMEVFS_LIBS) \
                      $(GCONF_LIBS)    \
                      $(LIBXML_LIBS)   \
                      $(LIBWNCK_LIBS)

libprobot_la_SOURCES = $(BUILT_SOURCES)    \
                       probot-utils.c      \
                       probot-app-entry.c  \
                       probot-controller.c

CLEANFILES = $(BUILT_SOURCES) *~ *.la *.o *.so *.lo *.loT *.pc
