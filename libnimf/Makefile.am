lib_LTLIBRARIES = libnimf.la

BUILT_SOURCES = \
	nimf-enum-types.h \
	nimf-enum-types.c \
	nimf-marshalers.h \
	nimf-marshalers.c \
	$(NULL)

libnimf_la_SOURCES = \
	nimf.h \
	nimf-client.h \
	nimf-client.c \
	nimf-private.h \
	nimf-private.c \
	nimf-im.c \
	nimf-im.h \
	nimf-types.c \
	nimf-types.h \
	nimf-events.c \
	nimf-events.h \
	nimf-engine.c \
	nimf-engine.h \
	nimf-module.h \
	nimf-module.c \
	nimf-key-syms.h \
	nimf-key-syms.c \
	nimf-message.h \
	nimf-message.c \
	nimf-connection.c \
	nimf-connection.h \
	nimf-candidatable.c \
	nimf-candidatable.h \
	nimf-preeditable.c \
	nimf-preeditable.h \
	nimf-server.c \
	nimf-server.h \
	nimf-server-im.c \
	nimf-server-im.h \
	nimf-service.c \
	nimf-service.h \
	nimf-service-im.c \
	nimf-service-im.h \
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
	nimf-client.h \
	nimf-connection.h \
	nimf-engine.h \
	nimf-enum-types.h \
	nimf-events.h \
	nimf-im.h \
	nimf-key-syms.h \
	nimf-message.h \
	nimf-preeditable.h \
	nimf-private.h \
	nimf-server.h \
	nimf-service.h \
	nimf-service-im.h \
	nimf-types.h

nimf-marshalers.h: nimf-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=nimf_cclosure_marshal \
	                            --header nimf-marshalers.list > nimf-marshalers.h

nimf-marshalers.c: nimf-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=nimf_cclosure_marshal \
	                            --body nimf-marshalers.list > nimf-marshalers.c

nimf-enum-types.h: nimf-key-syms.h \
	                 nimf-message.h \
	                 nimf-types.h \
	                 nimf-enum-types.h.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Nimf \
	                         --template nimf-enum-types.h.template \
	                           nimf-key-syms.h \
	                           nimf-message.h \
	                           nimf-types.h > nimf-enum-types.h

nimf-enum-types.c: nimf-key-syms.h \
	                 nimf-message.h \
	                 nimf-types.h \
	                 nimf-enum-types.c.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Nimf \
	                         --template nimf-enum-types.c.template \
	                           nimf-key-syms.h \
	                           nimf-message.h \
	                           nimf-types.h > nimf-enum-types.c

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
