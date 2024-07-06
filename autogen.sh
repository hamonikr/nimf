#!/bin/sh
# Run this to generate all the initial makefiles, etc.

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

# Clean up previous build files
echo "Cleaning up previous build files..."
make clean > /dev/null 2>&1
rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh

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

# Detect Ubuntu version
UBUNTU_VERSION=`lsb_release -r | awk '{print $2}'`

# Check if Qt6Core.pc exists
if ! pkg-config --exists Qt6Core; then
    # Check if Ubuntu version is 22.04 or 7.0
    if [ "$UBUNTU_VERSION" = "22.04" ] || [ "$UBUNTU_VERSION" = "7.0" ]; then
        echo "Creating missing Qt6 .pc files"

        QT6_FULL_VERSION=`dpkg -l | grep qt6-base-dev | awk '{print $3}' | head -1`
        QT6_VERSION=`echo $QT6_FULL_VERSION | grep -oP '^[0-9]+\.[0-9]+\.[0-9]+'`
        echo "Found Qt6 $QT6_VERSION"

        echo "prefix=/usr" > Qt6Core.pc
        echo "exec_prefix=\${prefix}" >> Qt6Core.pc
        echo "libdir=\${exec_prefix}/lib" >> Qt6Core.pc
        echo "includedir=\${prefix}/include/x86_64-linux-gnu/qt6" >> Qt6Core.pc
        echo "" >> Qt6Core.pc
        echo "Name: Qt6 Core" >> Qt6Core.pc
        echo "Description: Qt6 Core module" >> Qt6Core.pc
        echo "Version: $QT6_VERSION" >> Qt6Core.pc
        echo "Libs: -L\${libdir} -lQt6Core" >> Qt6Core.pc
        echo "Cflags: -I\${includedir}/QtCore" >> Qt6Core.pc
        sudo mv Qt6Core.pc /usr/lib/pkgconfig/

        echo "prefix=/usr" > Qt6Gui.pc
        echo "exec_prefix=\${prefix}" >> Qt6Gui.pc
        echo "libdir=\${exec_prefix}/lib" >> Qt6Gui.pc
        echo "includedir=\${prefix}/include/x86_64-linux-gnu/qt6" >> Qt6Gui.pc
        echo "" >> Qt6Gui.pc
        echo "Name: Qt6 Gui" >> Qt6Gui.pc
        echo "Description: Qt6 Gui module" >> Qt6Gui.pc
        echo "Version: $QT6_VERSION" >> Qt6Gui.pc
        echo "Libs: -L\${libdir} -lQt6Gui" >> Qt6Gui.pc
        echo "Cflags: -I\${includedir} -I\${includedir}/QtGui" >> Qt6Gui.pc
        sudo mv Qt6Gui.pc /usr/lib/pkgconfig/

        echo "prefix=/usr" > Qt6Widgets.pc
        echo "exec_prefix=\${prefix}" >> Qt6Widgets.pc
        echo "libdir=\${exec_prefix}/lib" >> Qt6Widgets.pc
        echo "includedir=\${prefix}/include/x86_64-linux-gnu/qt6" >> Qt6Widgets.pc
        echo "" >> Qt6Widgets.pc
        echo "Name: Qt6 Widgets" >> Qt6Widgets.pc
        echo "Description: Qt6 Widgets module" >> Qt6Widgets.pc
        echo "Version: $QT6_VERSION" >> Qt6Widgets.pc
        echo "Libs: -L\${libdir} -lQt6Widgets" >> Qt6Widgets.pc
        echo "Cflags: -I\${includedir} -I\${includedir}/QtWidgets" >> Qt6Widgets.pc
        sudo mv Qt6Widgets.pc /usr/lib/pkgconfig/
    fi
fi

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
    echo "autoreconf not found, please install autoconf package"
    exit 1
else
    autoreconf --force --install --verbose || exit $?
fi

cd "$olddir"
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@" --libdir=/usr/lib/x86_64-linux-gnu
