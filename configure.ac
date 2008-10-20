AC_INIT(genesis, 0.0.1)

AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_LIBTOOL

AC_C_CONST
AC_HEADER_STDC

AC_HEADER_STDBOO

#++++++++++++++++++
# Misc build flags
#++++++++++++++++++

AC_CHECK_LIB(gthread-2.0, g_thread_init)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

CFLAGS="$CFLAGS -Wall -ansi -Wmissing-prototypes -Wmissing-declarations -Werror -std=c99 -rdynamic"

AC_SUBST(CFLAGS)

#+++++++++++++++++++++
# Dependencies checks
#+++++++++++++++++++++

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6.0)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.6.0)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1)
AC_SUBST(DBUS_GLIB_LIBS)
AC_SUBST(DBUS_GLIB_CFLAGS)

DBUS_BINDING_TOOL="dbus-binding-tool"
AC_SUBST(DBUS_BINDING_TOOL)


PKG_CHECK_MODULES(LIBXML, libxml-2.0)
AC_SUBST(LIBXML_LIBS)
AC_SUBST(LIBXML_CFLAGS)

PKG_CHECK_MODULES(LIBWNCK, libwnck-1.0)
AC_SUBST(LIBWNCK_LIBS)
AC_SUBST(LIBWNCK_CFLAGS)

genesisdir=${libdir}
AC_SUBST(genesisdir)

AC_CONFIG_FILES([
Makefile
src/Makefile
src/genesis.pc
test/Makefile
])

AC_OUTPUT
