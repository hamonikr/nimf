moduledir = $(libdir)/nimf/modules/services
module_LTLIBRARIES = libnimf-candidate.la

libnimf_candidate_la_SOURCES = \
	nimf-candidate.c \
	$(NULL)

libnimf_candidate_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_CANDIDATE_DEPS_CFLAGS)

libnimf_candidate_la_LDFLAGS = -avoid-version -module $(NIMF_CANDIDATE_DEPS_LIBS)
libnimf_candidate_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-candidate.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-candidate.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-candidate.so
	-rmdir -p $(DESTDIR)$(moduledir)
