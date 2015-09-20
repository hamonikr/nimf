#!/bin/sh
# Run this to generate all the initial makefiles, etc.

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

mkdir -p m4

INTLTOOLIZE=`which intltoolize`
if test -z $INTLTOOLIZE=; then
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
