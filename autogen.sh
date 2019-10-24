#!/bin/sh
# Run this to generate all the initial makefiles, etc.

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

mkdir -p m4

PKGCONFIG=`which pkg-config`
if test -z "$PKGCONFIG"; then
    echo "pkg-config not found, please install pkg-config"
    exit 1
fi

pkg-config --print-errors glib-2.0
if [ $? != 0 ]; then
    echo "You probably need to install libglib2.0-dev or glib2-devel"
    exit 1
fi

LIBTOOLIZE=`which libtoolize`
if test -z $LIBTOOLIZE; then
    echo "libtoolize not found, please install libtool package"
    exit 1
fi

GTKDOCIZE=`which gtkdocize`
if test -z $GTKDOCIZE; then
    echo "gtkdocize not found, please install gtk-doc-tools or gtk-doc package"
    exit 1
else
    gtkdocize || exit $?
fi

INTLTOOLIZE=`which intltoolize`
if test -z $INTLTOOLIZE; then
    echo "intltoolize not found, please install intltool package"
    exit 1
else
    intltoolize --force --copy --automake || exit $?
fi

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
    echo "autoreconf not found, please install autoconf package"
    exit 1
else
    autoreconf --force --install --verbose || exit $?
fi

cd "$olddir"
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
