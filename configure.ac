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

AC_HEADER_STDBOOL

#++++++++++++++++++
# Misc build flags
#++++++++++++++++++

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

PKG_CHECK_MODULES(GNOMEVFS, gnome-vfs-2.0 >= 2.8.3)
AC_SUBST(GNOMEVFS_CFLAGS)
AC_SUBST(GNOMEVFS_LIBS)

PKG_CHECK_MODULES(GCONF, gconf-2.0 >= 2.6.2)
AC_SUBST(GCONF_LIBS)
AC_SUBST(GCONF_CFLAGS)

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
src/libgenesis.pc
test/Makefile
])

AC_OUTPUT
