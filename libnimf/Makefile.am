lib_LTLIBRARIES = libnimf.la

BUILT_SOURCES = \
	nimf-enum-types.c \
	nimf-enum-types-private.h \
	nimf-marshalers.c \
	nimf-marshalers-private.h \
	nimf-message-enum-types-private.h \
	nimf-message-enum-types.c \
	$(NULL)

libnimf_la_SOURCES = \
	nimf.h \
	nimf-candidatable.c \
	nimf-candidatable.h \
	nimf-engine.c \
	nimf-engine.h \
	nimf-events.c \
	nimf-events.h \
	nimf-im.c \
	nimf-im.h \
	nimf-key-syms.h \
	nimf-message.c \
	nimf-message-private.h \
	nimf-module.c \
	nimf-module-private.h \
	nimf-preeditable.c \
	nimf-preeditable.h \
	nimf-server.c \
	nimf-server.h \
	nimf-server-private.h \
	nimf-service.c \
	nimf-service.h \
	nimf-service-ic.c \
	nimf-service-ic.h \
	nimf-service-ic-private.h \
	nimf-types.c \
	nimf-types.h \
	nimf-utils.c \
	nimf-utils.h \
	nimf-utils-private.h \
	$(BUILT_SOURCES) \
	$(NULL)

libnimf_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	-DNIMF_SERVICE_MODULE_DIR=\"$(libdir)/nimf/modules/services\" \
	$(LIBNIMF_DEPS_CFLAGS)

libnimf_la_LDFLAGS = -version-info $(LIBNIMF_LT_VERSION) $(LIBNIMF_DEPS_LIBS)

nimfincludedir = $(includedir)/nimf
nimfinclude_HEADERS = \
	nimf.h \
	nimf-candidatable.h \
	nimf-engine.h \
	nimf-events.h \
	nimf-im.h \
	nimf-key-syms.h \
	nimf-preeditable.h \
	nimf-server.h \
	nimf-service.h \
	nimf-service-ic.h \
	nimf-types.h \
	nimf-utils.h \
	$(NULL)

nimf-marshalers-private.h: nimf-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=nimf_cclosure_marshal \
	                            --header nimf-marshalers.list \
	                            > nimf-marshalers-private.h

nimf-marshalers.c: nimf-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=nimf_cclosure_marshal \
	                            --body nimf-marshalers.list \
	                            > nimf-marshalers.c

nimf-enum-types-private.h: nimf-key-syms.h \
	                         nimf-types.h \
	                         nimf-enum-types-private.h.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Nimf \
	                         --template nimf-enum-types-private.h.template \
	                           nimf-key-syms.h nimf-types.h \
	                         > nimf-enum-types-private.h

nimf-enum-types.c: nimf-key-syms.h \
	                 nimf-types.h \
	                 nimf-enum-types.c.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Nimf \
	                         --template nimf-enum-types.c.template \
	                           nimf-key-syms.h nimf-types.h \
	                         > nimf-enum-types.c

nimf-message-enum-types-private.h: \
	nimf-message-private.h \
	nimf-message-enum-types-private.h.template
	$(AM_V_GEN) glib-mkenums \
		--identifier-prefix Nimf \
		--template nimf-message-enum-types-private.h.template \
		nimf-message-private.h > nimf-message-enum-types-private.h

nimf-message-enum-types.c: nimf-message.c nimf-enum-types.c.template
	$(AM_V_GEN) glib-mkenums \
		--identifier-prefix Nimf \
		--template nimf-message-enum-types.c.template \
		nimf-message-private.h > nimf-message-enum-types.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nimf.pc

gsettings_SCHEMAS = org.nimf.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = nimf.pc.in

install-data-hook:
	rm -f $(DESTDIR)$(libdir)/libnimf.la

uninstall-hook:
	-rm    -f $(DESTDIR)$(libdir)/libnimf.so*
	-rmdir -p $(DESTDIR)$(libdir)/nimf
	-rmdir -p $(DESTDIR)$(nimfincludedir)

CLEANFILES = $(BUILT_SOURCES)

DISTCLEANFILES = Makefile.in
