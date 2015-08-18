dnl Process this file with autoconf to produce a configure script.

AC_INIT(dasom, 0.0.1)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.11 -Wall -Werror])

AM_SILENT_RULES([yes])

AM_PROG_AR
AC_PROG_CC
AC_PROG_CXX

GETTEXT_PACKAGE=dasom
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext package name])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_DEFINE_LOCALEDIR(DASOM_LOCALE_DIR)

LT_INIT([disable-static])

PKG_CHECK_MODULES(LIBDASOM_DEPS,
                  [glib-2.0 gio-2.0 gio-unix-2.0 gmodule-2.0
                   gobject-introspection-1.0])

PKG_CHECK_MODULES(GTK3, [gtk+-3.0])
PKG_CHECK_MODULES(GTK2, [gtk+-2.0])
PKG_CHECK_MODULES(LIBHANGUL, libhangul >= 0.0.12)
PKG_CHECK_MODULES(X11, [x11])
PKG_CHECK_MODULES(QT4_GUI, [QtGui])
PKG_CHECK_MODULES(QT5_PLATFORM_SUPPORT, [Qt5PlatformSupport])

QT5_HOST_BINS=`pkg-config --variable=host_bins Qt5Core`
QT5_MOC=`which $QT5_HOST_BINS/moc`

AC_MSG_CHECKING([checking for $QT5_HOST_BINS/moc])
if test -z $QT5_MOC; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([moc for qt5 is required to build libqt5im-dasom.so])
else
  AC_MSG_RESULT(yes)
fi

AC_ARG_ENABLE([appindicator],
  AS_HELP_STRING([--enable-appindicator],
                 [Enable dasom-indicator (appindicator)]),
  [],
  [enable_appindicator=yes])

AS_IF([test "x$enable_appindicator" == "xyes"],
  [PKG_CHECK_MODULES(APPINDICATOR3, [appindicator3-0.1])
])

AM_CONDITIONAL([ENABLE_APPINDICATOR], [test "$enable_appindicator" = "yes"])

GLIB_GSETTINGS
GOBJECT_INTROSPECTION_CHECK([0.10.1])

AC_OUTPUT([
  Makefile
  agents/Makefile
  agents/gnome-shell-extension/Makefile
  agents/app-indicator/Makefile
  im-config-data/Makefile
  libdasom/Makefile
  libdasom/IMdkit/Makefile
  libdasom/dasom.pc
  modules/Makefile
  modules/clients/Makefile
  modules/clients/gtk/Makefile
  modules/clients/qt4/Makefile
  modules/clients/qt5/Makefile
  modules/engines/Makefile
  modules/engines/dasom-english/Makefile
  modules/engines/dasom-jeongeum/Makefile
  daemon/Makefile
  tests/Makefile])
