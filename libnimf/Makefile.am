lib_LTLIBRARIES = libnimf.la

BUILT_SOURCES = \
	nimf-enum-types.h \
	nimf-enum-types.c \
	nimf-marshalers.h \
	nimf-marshalers.c \
	$(NULL)

libnimf_la_SOURCES = \
	nimf.h \
	nimf-events.c \
	nimf-events.h \
	nimf-key-syms.c \
	nimf-key-syms.h \
	nimf-message.c \
	nimf-message.h \
	nimf-private.c \
	nimf-private.h \
	nimf-types.c \
	nimf-types.h \
	client/nimf-client.c \
	client/nimf-client.h \
	client/nimf-im.c \
	client/nimf-im.h \
	server/nimf-candidatable.c \
	server/nimf-candidatable.h \
	server/nimf-connection.c \
	server/nimf-connection.h \
	server/nimf-engine.c \
	server/nimf-engine.h \
	server/nimf-module.c \
	server/nimf-module.h \
	server/nimf-preeditable.c \
	server/nimf-preeditable.h \
	server/nimf-server.c \
	server/nimf-server.h \
	server/nimf-server-im.c \
	server/nimf-server-im.h \
	server/nimf-service.c \
	server/nimf-service.h \
	server/nimf-service-im.c \
	server/nimf-service-im.h \
	$(BUILT_SOURCES) \
	$(NULL)

libnimf_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(LIBNIMF_DEPS_CFLAGS)

libnimf_la_LDFLAGS = -version-info $(LIBNIMF_LT_VERSION) $(LIBNIMF_DEPS_LIBS)

nimfincludedir = $(includedir)/nimf
nimfinclude_HEADERS = \
	nimf.h \
	nimf-enum-types.h \
	nimf-events.h \
	nimf-key-syms.h \
	nimf-message.h \
	nimf-private.h \
	nimf-types.h \
	client/nimf-client.h \
	client/nimf-im.h \
	server/nimf-candidatable.h \
	server/nimf-connection.h \
	server/nimf-engine.h \
	server/nimf-preeditable.h \
	server/nimf-server.h \
	server/nimf-service.h \
	server/nimf-service-im.h \
	$(NULL)

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
