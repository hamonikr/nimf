gsettings_SCHEMAS = org.nimf.clients.qt6.gschema.xml
@GSETTINGS_RULES@

# Use the QT6_CFLAGS and QT6_LIBS variables from configure.ac
AM_CPPFLAGS = $(QT6_CFLAGS)
LDADD = $(QT6_LIBS)

qt6_im_moduledir = $(QT6_IM_MODULE_DIR)
qt6_im_module_LTLIBRARIES = libqt6im-nimf.la

BUILT_SOURCES = im-nimf-qt6.moc
libqt6im_nimf_la_SOURCES = im-nimf-qt6.cpp $(BUILT_SOURCES)

libqt6im_nimf_la_CXXFLAGS = \
	$(EXTRA_CFLAGS) \
	$(LIBNIMF_DEPS_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_COMPILATION \
	$(IM_NIMF_QT6_DEPS_CFLAGS) \
	-I$(QT6_CORE_PRIVATE_INCLUDE_PATH) \
	-I$(QT6_GUI_PRIVATE_INCLUDE_PATH) \
	-DQT_NO_KEYWORDS \
	-fPIC \
	-std=c++17

libqt6im_nimf_la_LDFLAGS = -avoid-version -module
libqt6im_nimf_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la $(IM_NIMF_QT6_DEPS_LIBS)

im-nimf-qt6.moc: im-nimf-qt6.cpp
	$(AM_V_GEN) $(QT6_MOC) -I $(QT6_CORE_PRIVATE_INCLUDE_PATH) \
	                       -I $(QT6_GUI_PRIVATE_INCLUDE_PATH) im-nimf-qt6.cpp \
	                       -o im-nimf-qt6.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt6_im_moduledir)/libqt6im-nimf.so
	rm -f    $(DESTDIR)$(qt6_im_moduledir)/libqt6im-nimf.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(qt6_im_moduledir)/libqt6im-nimf.so
	-rmdir -p $(DESTDIR)$(qt6_im_moduledir)

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
