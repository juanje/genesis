bin_PROGRAMS = probot_test

probot_test_CPPFLAGS = $(GLIB_CFLAGS)     \
                       $(GTK_CFLAGS)      \
                       $(GNOMEVFS_CFLAGS) \
                       $(GCONF_CFLAGS)    \
                       $(LIBXML_CFLAGS)   \
                       $(LIBWNCK_CFLAGS)  \
                       -I../src/

probot_test_LDFLAGS = ../src/.libs/libprobot.so \
                      $(GLIB_LIBS)              \
                      $(GNOMEVFS_LIBS)          \
                      $(GTK_LIBS)               \
                      $(LIBWNCK_LIBS)

probot_test_SOURCES = main.c
