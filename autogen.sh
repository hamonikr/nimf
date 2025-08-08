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

# OS detection and handling
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        OS_VERSION=$VERSION_ID
    else
        OS=`uname -s`
        OS_VERSION=`uname -r`
    fi
}

handle_fedora() {
    if pkg-config --exists "libhangul >= 0.1.0"; then
        echo "libhangul is already installed. Skipping installation."
    else
        echo "Detected Fedora. Installing libhangul..."
        cd libhangul
        ./autogen.sh
        ./configure --prefix=/usr
        make
        sudo make install
        cd "$srcdir"
    fi
}

handle_ubuntu() {
    echo "Creating missing Qt6 .pc files"
    QT6_FULL_VERSION=`dpkg -l | grep qt6-base-dev | awk '{print $3}' | head -1`
    QT6_VERSION=`echo $QT6_FULL_VERSION | grep -oP '^[0-9]+\.[0-9]+\.[0-9]+'`
    echo "Found Qt6 $QT6_VERSION"

    for module in Core Gui Widgets; do
        echo "prefix=/usr" > Qt6$module.pc
        echo "exec_prefix=\${prefix}" >> Qt6$module.pc
        echo "libdir=\${exec_prefix}/lib" >> Qt6$module.pc
        echo "includedir=\${prefix}/include/x86_64-linux-gnu/qt6" >> Qt6$module.pc
        echo "" >> Qt6$module.pc
        echo "Name: Qt6 $module" >> Qt6$module.pc
        echo "Description: Qt6 $module module" >> Qt6$module.pc
        echo "Version: $QT6_VERSION" >> Qt6$module.pc
        echo "Libs: -L\${libdir} -lQt6$module" >> Qt6$module.pc
        echo "Cflags: -I\${includedir} -I\${includedir}/Qt$module" >> Qt6$module.pc
        sudo mv Qt6$module.pc /usr/lib/pkgconfig/
    done
}

check_version_and_handle() {
    if [ "$OS" = "ubuntu" ] && [ "$OS_VERSION" = "22.04" ]; then
        handle_ubuntu
    elif [ "$OS" = "hamonikr" ] && [ "$OS_VERSION" = "7.0" ]; then
        handle_ubuntu
    fi
}

detect_os

if [ "$OS" = "fedora" ]; then
    handle_fedora
fi

check_version_and_handle

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
    echo "autoreconf not found, please install autoconf package"
    exit 1
else
    autoreconf --force --install --verbose || exit $?
fi

cd "$olddir"
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@" --prefix=/usr --libdir=/usr/lib/x86_64-linux-gnu
