dnl Process this file with autoconf to produce a configure script.

AC_INIT(dasom, 0.0.1)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.11 -Wall -Werror])

AM_SILENT_RULES([yes])

AM_PROG_AR
AC_PROG_CC

GETTEXT_PACKAGE=dasom
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext package name])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_DEFINE_LOCALEDIR(DASOM_LOCALE_DIR)

LT_INIT([disable-static])

PKG_CHECK_MODULES(GLIB, [glib-2.0])
PKG_CHECK_MODULES(GIO, [gio-2.0])
PKG_CHECK_MODULES(GIO_UNIX, [gio-unix-2.0])
PKG_CHECK_MODULES(GMODULE, [gmodule-2.0])
PKG_CHECK_MODULES(GTK3, [gtk+-3.0])
PKG_CHECK_MODULES(GTK2, [gtk+-2.0])
PKG_CHECK_MODULES(LIBHANGUL, libhangul >= 0.0.12)
PKG_CHECK_MODULES(X11, [x11])

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
  modules/engines/Makefile
  modules/engines/dasom-english/Makefile
  modules/engines/dasom-jeongeum/Makefile
  daemon/Makefile
  tests/Makefile])
