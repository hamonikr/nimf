dnl Process this file with autoconf to produce a configure script.

# 추가된 경로 설정
libdir='/usr/lib/x86_64-linux-gnu'

AC_SUBST(libdir)

AC_INIT(nimf, 1.3.5)

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])
AM_MAINTAINER_MODE([disable])
AM_SILENT_RULES([yes])

AM_PROG_AR
AC_PROG_CC
AC_PROG_CXX

AS_IF([test "$CXX" = "g++"],
      [AS_IF([test "$GXX" != "yes"],
      AC_MSG_ERROR([C++ compiler is not found.]))])

AC_ARG_ENABLE([hardening],
  AS_HELP_STRING([--disable-hardening], [Disable hardening]))

# EXTRA_CFLAGS="-Wall -Werror"
EXTRA_CFLAGS="-Wall"
AS_IF([test "x$enable_hardening" != "xno"],
      [AS_IF([echo "$EXTRA_CFLAGS" | grep -vq '\-D_FORTIFY_SOURCE'],
             [EXTRA_CFLAGS+=" -D_FORTIFY_SOURCE=2"])])

AC_SUBST(EXTRA_CFLAGS)

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************

IT_PROG_INTLTOOL(0.50.1)
GETTEXT_PACKAGE=nimf
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext package name])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(NIMF_LOCALE_DIR)

LT_INIT([disable-static])

dnl ***************************************************************************
dnl libnimf
dnl ***************************************************************************
# Before making a release, the LT_VERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A

LIBNIMF_LT_VERSION=1:0:0
AC_SUBST(LIBNIMF_LT_VERSION)

LIBNIMF_REQUIRES="glib-2.0 gio-2.0 gio-unix-2.0 gmodule-2.0"
AC_SUBST(LIBNIMF_REQUIRES)
PKG_CHECK_MODULES(LIBNIMF_DEPS, [$LIBNIMF_REQUIRES])

dnl ***************************************************************************
dnl nimf-libhangul  nimf-system-keyboard
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_LIBHANGUL_DEPS, [libhangul >= 0.0.12 $LIBNIMF_REQUIRES])
PKG_CHECK_MODULES(NIMF_SYSTEM_KEYBOARD_DEPS,
                                       [xkbcommon >= 0.5.0  $LIBNIMF_REQUIRES])

dnl ***************************************************************************
dnl nimf-m17n
dnl ***************************************************************************

AC_ARG_ENABLE([nimf-m17n],
  AS_HELP_STRING([--disable-nimf-m17n], [Disable nimf-m17n]))

AS_IF([test "x$enable_nimf_m17n" != "xno"],
      [PKG_CHECK_MODULES(NIMF_M17N_DEPS, [m17n-shell >= 1.7.0 m17n-db >= 1.7.0
                                          $LIBNIMF_REQUIRES])])

AM_CONDITIONAL([ENABLE_NIMF_M17N], [test "x$enable_nimf_m17n" != "xno"])

PKG_CHECK_EXISTS([m17n-db >= 1.8.0], [m17n_1_8_0_available="yes"])
AM_CONDITIONAL([M17N_1_8_0_AVAILABLE], [test "x$m17n_1_8_0_available" = "xyes"])

dnl ***************************************************************************
dnl nimf-rime
dnl ***************************************************************************

AC_ARG_ENABLE([nimf-rime],
              AS_HELP_STRING([--disable-nimf-rime], [Disable nimf-rime]))

AS_IF([test "x$enable_nimf_rime" != "xno"],
      [PKG_CHECK_MODULES(NIMF_RIME_DEPS, [rime >= 1.2.9 $LIBNIMF_REQUIRES])])

AM_CONDITIONAL([ENABLE_NIMF_RIME], [test "x$enable_nimf_rime" != "xno"])

dnl ***************************************************************************
dnl nimf-anthy
dnl ***************************************************************************

AC_ARG_ENABLE([nimf-anthy],
  AS_HELP_STRING([--disable-nimf-anthy], [Disable nimf-anthy]))

AS_IF([test "x$enable_nimf_anthy" != "xno"],
      [PKG_CHECK_MODULES(NIMF_ANTHY_DEPS, [anthy $LIBNIMF_REQUIRES])])
AS_IF([test "x$enable_nimf_anthy" != "xno"],
      [AC_CHECK_FILE([/var/lib/anthy/anthy.dic], [HAVE_ANTHY_DIC="yes"])])
AS_IF([test "x$enable_nimf_anthy" != "xno"],
      [AC_CHECK_FILE([/usr/share/anthy/anthy.dic], [HAVE_ANTHY_DIC="yes"])])
AS_IF([test "x$enable_nimf_anthy" != "xno" -a "x$HAVE_ANTHY_DIC" != "xyes"],
      [AC_MSG_ERROR([anthy.dic is not found.])])

AM_CONDITIONAL([ENABLE_NIMF_ANTHY], [test "x$enable_nimf_anthy" != "xno"])

dnl ***************************************************************************
dnl im-nimf-qt5
dnl ***************************************************************************

# Check for Qt5
PKG_CHECK_MODULES(IM_NIMF_QT5_DEPS, [Qt5Core Qt5Gui Qt5Widgets], [has_qt5=yes], [has_qt5=no])
AS_IF([test "x$has_qt5" = "xyes"], [
  QT5_VERSION=`pkg-config --modversion Qt5Core`
  QT5_INCLUDE_DIR=`pkg-config --variable=includedir Qt5Core`
  QT5_CORE_PRIVATE_INCLUDE_PATH=$QT5_INCLUDE_DIR/QtCore/$QT5_VERSION
  QT5_GUI_PRIVATE_INCLUDE_PATH=$QT5_INCLUDE_DIR/QtGui/$QT5_VERSION

  AC_CHECK_FILES(
     [$QT5_GUI_PRIVATE_INCLUDE_PATH/QtGui/qpa/qplatforminputcontext.h
      $QT5_GUI_PRIVATE_INCLUDE_PATH/QtGui/qpa/qplatforminputcontextplugin_p.h], [],
      AC_MSG_ERROR([Qt 5 base private development files are not found.]))

  AC_SUBST(QT5_CORE_PRIVATE_INCLUDE_PATH)
  AC_SUBST(QT5_GUI_PRIVATE_INCLUDE_PATH)

  AC_PATH_PROGS([QT5_MOC], [moc-qt5 moc], [no],
                [`pkg-config --variable=host_bins Qt5Core`])
  if test "x$QT5_MOC" = "xno"; then
    AC_MSG_ERROR([moc for qt5 not found])
  fi

  QT5_LIB_DIR=`pkg-config --variable=libdir Qt5Core`

  case `pkg-config --variable=includedir Qt5Core` in
    */qt5)
      QT5_IM_MODULE_DIR=$QT5_LIB_DIR/qt5/plugins/platforminputcontexts
      ;;
    */qt)
      QT5_IM_MODULE_DIR=$QT5_LIB_DIR/qt/plugins/platforminputcontexts
      ;;
    *)
      AC_MSG_ERROR(Can not determine Qt5 plugins/platforminputcontexts directory)
      ;;
  esac

  AC_SUBST(QT5_IM_MODULE_DIR)
])

dnl ***************************************************************************
dnl im-nimf-qt6
dnl ***************************************************************************

PKG_CHECK_MODULES(IM_NIMF_QT6_DEPS, [Qt6Core Qt6Gui Qt6Widgets], [has_qt6=yes], [has_qt6=no])
AS_IF([test "x$has_qt6" = "xyes"], [
  QT6_VERSION=`pkg-config --modversion Qt6Core`
  QT6_INCLUDE_DIR=`pkg-config --variable=includedir Qt6Core`
  QT6_CORE_PRIVATE_INCLUDE_PATH=$QT6_INCLUDE_DIR/QtCore/$QT6_VERSION
  QT6_GUI_PRIVATE_INCLUDE_PATH=$QT6_INCLUDE_DIR/QtGui/$QT6_VERSION

  # Add Qt6 checks
  AC_CHECK_HEADERS([QtCore/QtCore,
                    QtGui/QtGui,
                    QtWidgets/QtWidgets,
                    QtGui/6.4.2/QtGui/qpa/qplatforminputcontext.h])

  AC_CHECK_FILES(
     [$QT6_GUI_PRIVATE_INCLUDE_PATH/QtGui/qpa/qplatforminputcontext.h
      $QT6_GUI_PRIVATE_INCLUDE_PATH/QtGui/qpa/qplatforminputcontextplugin_p.h], [],
      AC_MSG_ERROR([Qt 6 base private development files are not found.]))

  AC_SUBST(QT6_CORE_PRIVATE_INCLUDE_PATH)
  AC_SUBST(QT6_GUI_PRIVATE_INCLUDE_PATH)

  AC_PATH_PROGS([QT6_MOC], [moc-qt6 moc], [no],
                [`pkg-config --variable=host_bins Qt6Core`])
  if test "x$QT6_MOC" = "xno"; then
    QT6_MOC=`find /usr/lib/ -name moc -type f | grep qt6`
    if test "x$QT6_MOC" = "x"; then
      AC_MSG_ERROR([moc for qt6 not found])
    fi
  fi

  QT6_LIB_DIR=`pkg-config --variable=libdir Qt6Core`

  case `pkg-config --variable=includedir Qt6Core` in
    */qt6)
      QT6_IM_MODULE_DIR=$QT6_LIB_DIR/qt6/plugins/platforminputcontexts
      ;;
    */qt)
      QT6_IM_MODULE_DIR=$QT6_LIB_DIR/qt/plugins/platforminputcontexts
      ;;
    *)
      AC_MSG_ERROR(Can not determine Qt6 plugins/platforminputcontexts directory)
      ;;
  esac

  AC_SUBST(QT6_IM_MODULE_DIR)
])

AM_CONDITIONAL([HAVE_QT5], [test "x$has_qt5" = "xyes"])
AM_CONDITIONAL([HAVE_QT6], [test "x$has_qt6" = "xyes"])


dnl ***************************************************************************
dnl  Check for im-nimf-gtk
dnl ***************************************************************************

PKG_CHECK_MODULES(IM_NIMF_GTK3_DEPS, [gtk+-3.0 gdk-x11-3.0])
PKG_CHECK_MODULES(IM_NIMF_GTK2_DEPS, [gtk+-2.0])

case `uname -m` in
  x86_64)
    BITS=64
    ;;
  *)
    BITS=32
    ;;
esac

GTK3_LIBDIR=`pkg-config --variable=libdir gtk+-3.0`
GTK3_BINARY_VERSION=`pkg-config --variable=gtk_binary_version gtk+-3.0`
AC_SUBST(GTK3_LIBDIR)
AC_SUBST(GTK3_BINARY_VERSION)
AC_PATH_PROGS([GTK_QUERY_IMMODULES3],
              [gtk-query-immodules-3.0 gtk-query-immodules-3.0-64], [no],
              [/usr/bin:/snap/gnome-42-2204/176/usr/lib/x86_64-linux-gnu/libgtk-3-0:/usr/lib/x86_64-linux-gnu/libgtk-3-0t64:/usr/lib/x86_64-linux-gnu/libgtk-3-0])
if test "x$GTK_QUERY_IMMODULES3" = "xno"; then
  AC_MSG_ERROR([gtk-query-immodules-3.0 or gtk-query-immodules-3.0-64 not found])
fi

AC_PATH_PROGS([GTK_QUERY_IMMODULES2],
              [gtk-query-immodules-2.0 gtk-query-immodules-2.0-64], [no],
              [/usr/bin:/snap/gnome-42-2204/176/usr/lib/x86_64-linux-gnu/libgtk2.0-0:/usr/lib/x86_64-linux-gnu/libgtk2.0-0t64:/usr/lib/x86_64-linux-gnu/libgtk2.0-0])
if test "x$GTK_QUERY_IMMODULES2" = "xno"; then
  AC_MSG_ERROR([gtk-query-immodules-2.0 or gtk-query-immodules-2.0-64 not found])
fi


dnl ***************************************************************************
dnl  icon cache
dnl ***************************************************************************

AC_PATH_PROGS([GTK_UPDATE_ICON_CACHE],
              [gtk-update-icon-cache-3.0 gtk-update-icon-cache], [no],
              [/usr/bin:$GTK3_LIBDIR/libgtk-3-0:$GTK2_LIBDIR/libgtk2.0-0])
if test "x$GTK_UPDATE_ICON_CACHE" = "xno"; then
  AC_MSG_ERROR([gtk-update-icon-cache-3.0 or gtk-update-icon-cache not found])
fi

AC_PATH_PROGS([RSVG_CONVERT], [rsvg-convert rsvg], [no])
if test "x$RSVG_CONVERT" = "xno"; then
  AC_MSG_ERROR([rsvg-convert or rsvg is not found, please install librsvg2-bin])
fi

dnl ***************************************************************************
dnl nimf-wayland
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_WAYLAND_DEPS, [$LIBNIMF_REQUIRES]
                                     wayland-client wayland-server
                                     wayland-protocols xkbcommon)

dnl ***************************************************************************
dnl nimf-xim
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_XIM_DEPS, [$LIBNIMF_REQUIRES] x11)

dnl ***************************************************************************
dnl nimf-nim
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_NIM_DEPS, [$LIBNIMF_REQUIRES] glib-2.0 gio-2.0)

dnl ***************************************************************************
dnl nimf server
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_DEPS, [glib-2.0 gio-2.0 gobject-2.0 gmodule-2.0])

dnl ***************************************************************************
dnl nimf-indicator
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_INDICATOR_DEPS,
                  [glib-2.0 x11 gtk+-3.0 ayatana-appindicator3-0.1 libxklavier gdk-3.0
                   gdk-x11-3.0 gmodule-2.0])

dnl ***************************************************************************
dnl nimf-preedit-window  nimf-candidate
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_PREEDIT_WINDOW_DEPS, [glib-2.0 gtk+-3.0])
PKG_CHECK_MODULES(NIMF_CANDIDATE_DEPS,      [glib-2.0 gtk+-3.0])

dnl ***************************************************************************
dnl nimf-settings
dnl ***************************************************************************

PKG_CHECK_MODULES(NIMF_SETTINGS_DEPS,
                  [gtk+-3.0 glib-2.0 >= 2.54 gio-2.0 gmodule-2.0 libxklavier
                   gdk-3.0 gdk-x11-3.0])

dnl ***************************************************************************
dnl im-config data
dnl ***************************************************************************

AC_ARG_WITH([im-config-data],
  AS_HELP_STRING([--with-im-config-data], [Install im-config data]),
  [with_im_config_data=yes])

AM_CONDITIONAL([WITH_IM_CONFIG_DATA],
               [test "$with_im_config_data" = "yes"])

dnl ***************************************************************************
dnl imsettings data
dnl ***************************************************************************

AC_ARG_WITH([imsettings-data],
  AS_HELP_STRING([--with-imsettings-data], [Install imsettings data]),
  [with_imsettings_data=yes])

AM_CONDITIONAL([WITH_IMSETTINGS_DATA],
               [test "$with_imsettings_data" = "yes"])

dnl ***************************************************************************
dnl Check for gtk-doc
dnl ***************************************************************************

GTK_DOC_CHECK([1.26],[--flavour no-tmpl])

dnl ***************************************************************************

GLIB_GSETTINGS

AC_OUTPUT([
  Makefile
  bin/Makefile
  bin/nimf/Makefile
  bin/nimf-settings/Makefile
  data/Makefile
  data/apparmor-abstractions/Makefile
  data/icons/Makefile
  data/im-config/Makefile
  data/imsettings/Makefile
  docs/Makefile
  libnimf/Makefile
  libnimf/nimf.pc
  modules/Makefile
  modules/clients/Makefile
  modules/clients/gtk/Makefile
  modules/clients/qt5/Makefile
  modules/clients/qt6/Makefile  
  modules/engines/Makefile
  modules/engines/nimf-anthy/Makefile
  modules/engines/nimf-libhangul/Makefile
  modules/engines/nimf-m17n/Makefile
  modules/engines/nimf-m17n/icons/Makefile
  modules/engines/nimf-rime/Makefile
  modules/engines/nimf-system-keyboard/Makefile
  modules/services/Makefile
  modules/services/candidate/Makefile
  modules/services/indicator/Makefile
  modules/services/nim/Makefile
  modules/services/preedit-window/Makefile
  modules/services/wayland/Makefile
  modules/services/xim/Makefile
  po/Makefile.in
])
