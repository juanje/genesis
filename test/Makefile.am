bin_PROGRAMS = genesis_test

genesis_test_CPPFLAGS = $(GLIB_CFLAGS)     \
                        $(GTK_CFLAGS)      \
                        $(GNOMEVFS_CFLAGS) \
                        $(GCONF_CFLAGS)    \
                        $(LIBXML_CFLAGS)   \
                        $(LIBWNCK_CFLAGS)  \
                        -I../src/

genesis_test_LDFLAGS = ../src/.libs/libgenesis.so \
                       $(GLIB_LIBS)               \
                       $(GNOMEVFS_LIBS)           \
                       $(GTK_LIBS)                \
                       $(LIBWNCK_LIBS)

genesis_test_SOURCES = main.c
