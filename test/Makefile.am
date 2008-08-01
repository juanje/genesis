COMMON_CFLAGS = $(GLIB_CFLAGS)     \
                $(GTK_CFLAGS)      \
                $(GNOMEVFS_CFLAGS) \
                $(GCONF_CFLAGS)    \
                $(LIBXML_CFLAGS)   \
                $(LIBWNCK_CFLAGS)  \
                -I../src/

COMMON_LDFLAGS = ../src/.libs/libgenesis.so \
                 $(GLIB_LIBS)               \
                 $(GNOMEVFS_LIBS)           \
                 $(GTK_LIBS)                \
                 $(LIBWNCK_LIBS)

bin_PROGRAMS = app_entry_test fs_monitor_test
app_entry_test_CPPFLAGS = $(COMMON_CFLAGS)

app_entry_test_LDFLAGS = $(COMMON_LDFLAGS)

app_entry_test_SOURCES = app-entry-test.c

fs_monitor_test_CPPFLAGS = $(COMMON_CFLAGS)

fs_monitor_test_LDFLAGS = $(COMMON_LDFLAGS)

fs_monitor_test_SOURCES = fs-monitor-test.c
