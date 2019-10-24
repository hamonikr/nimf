moduledir = $(libdir)/nimf/modules/services
module_LTLIBRARIES = libnimf-xim.la

IMdkit_SOURCES = \
	IMdkit/FrameMgr.h \
	IMdkit/Xi18n.h \
	IMdkit/XimFunc.h \
	IMdkit/XimProto.h \
	IMdkit/FrameMgr.c \
	IMdkit/i18nAttr.c \
	IMdkit/i18nClbk.c \
	IMdkit/i18nIc.c \
	IMdkit/i18nIMProto.c \
	IMdkit/i18nMethod.c \
	IMdkit/i18nMethod.h \
	IMdkit/i18nOffsetCache.c \
	IMdkit/i18nPtHdr.c \
	IMdkit/i18nUtil.c \
	IMdkit/i18nX.c \
	IMdkit/i18nX.h \
	$(NULL)

libnimf_xim_la_SOURCES = \
	nimf-xim.c \
	nimf-xim.h \
	nimf-xim-ic.c \
	nimf-xim-ic.h \
	$(IMdkit_SOURCES) \
	$(NULL)

libnimf_xim_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_XIM_DEPS_CFLAGS)

libnimf_xim_la_LDFLAGS = -avoid-version -module $(NIMF_XIM_DEPS_LIBS)
libnimf_xim_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-xim.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-xim.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-xim.so
	-rmdir -p $(DESTDIR)$(moduledir)
