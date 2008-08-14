COMMON_CFLAGS = $(GLIB_CFLAGS)     \
                $(GTK_CFLAGS)      \
                $(LIBXML_CFLAGS)   \
                $(LIBWNCK_CFLAGS)  \
                -I../src/

COMMON_LDFLAGS = $(GLIB_LIBS)               \
                 $(GTK_LIBS)                \
                 $(LIBWNCK_LIBS)            \
                 ../src/.libs/libgenesis.so

bin_PROGRAMS = app_entry_test
app_entry_test_CPPFLAGS = $(COMMON_CFLAGS)

app_entry_test_LDFLAGS = $(COMMON_LDFLAGS)

app_entry_test_SOURCES = app-entry-test.c
