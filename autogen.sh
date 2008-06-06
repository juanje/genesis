#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

set -x
libtoolize --automake
aclocal
autoconf
autoheader
automake --copy --add-missing --foreign

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME || exit 1
else
  echo Skipping configure process.
fi

