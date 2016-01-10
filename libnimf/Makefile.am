lib_LTLIBRARIES = libnimf.la

BUILT_SOURCES =      \
	nimf-enum-types.h \
	nimf-enum-types.c \
	nimf-marshalers.h \
	nimf-marshalers.c \
	$(NULL)

IMdkit_SOURCES = \
	IMdkit/FrameMgr.h \
	IMdkit/IMdkit.h \
	IMdkit/Xi18n.h \
	IMdkit/Xi18nX.h \
	IMdkit/XimFunc.h \
	IMdkit/XimProto.h \
	IMdkit/FrameMgr.c \
	IMdkit/i18nAttr.c \
	IMdkit/i18nClbk.c \
	IMdkit/i18nIc.c \
	IMdkit/i18nIMProto.c \
	IMdkit/i18nMethod.c \
	IMdkit/i18nOffsetCache.c \
	IMdkit/i18nPtHdr.c \
	IMdkit/i18nUtil.c \
	IMdkit/i18nX.c \
	IMdkit/IMConn.c \
	IMdkit/IMMethod.c \
	IMdkit/IMValues.c \
	$(NULL)

libnimf_la_SOURCES = \
	nimf.h \
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
	nimf-english.c \
	nimf-english.h \
	nimf-module.h \
	nimf-module.c \
	nimf-module-manager.h \
	nimf-module-manager.c \
	nimf-key-syms.h \
	nimf-message.h \
	nimf-message.c \
	nimf-candidate.h \
	nimf-candidate.c \
	nimf-agent.c \
	nimf-agent.h \
	nimf-connection.c \
	nimf-connection.h \
	nimf-server.c \
	nimf-server.h \
	$(BUILT_SOURCES) \
	$(IMdkit_SOURCES) \
	$(NULL)

libnimf_la_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	$(LIBNIMF_DEPS_CFLAGS)

libnimf_la_LDFLAGS = $(LIBNIMF_DEPS_LIBS) -avoid-version

nimfincludedir = $(includedir)/nimf
nimfinclude_HEADERS = \
	nimf.h \
	nimf-agent.h \
	nimf-candidate.h \
	nimf-connection.h \
	nimf-engine.h \
	nimf-english.h \
	nimf-enum-types.h \
	nimf-events.h \
	nimf-im.h \
	nimf-key-syms.h \
	nimf-message.h \
	nimf-module-manager.h \
	nimf-private.h \
	nimf-server.h \
	nimf-types.h

nimf-marshalers.h: nimf-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=nimf_cclosure_marshal \
	                            --header nimf-marshalers.list > nimf-marshalers.h

nimf-marshalers.c: nimf-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=nimf_cclosure_marshal \
	                            --body nimf-marshalers.list > nimf-marshalers.c

nimf-enum-types.h: nimf-message.h nimf-enum-types.h.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Nimf \
	                         --template nimf-enum-types.h.template \
	                           nimf-message.h nimf-key-syms.h nimf-types.h > nimf-enum-types.h

nimf-enum-types.c: nimf-message.h nimf-enum-types.c.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Nimf \
	                         --template nimf-enum-types.c.template \
	                           nimf-message.h nimf-key-syms.h nimf-types.h > nimf-enum-types.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nimf.pc

gsettings_SCHEMAS = org.nimf.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = nimf.pc.in

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS  = --add-include-path=$(srcdir) \
                              --warn-all --warn-error \
                              --identifier-prefix=Nimf --symbol-prefix=nimf
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = nimf-agent.c nimf-agent.h

Nimf-0.1.gir: libnimf.la
Nimf_0_1_gir_INCLUDES = GObject-2.0
Nimf_0_1_gir_CFLAGS   = $(libnimf_la_CFLAGS) -I$(top_srcdir)
Nimf_0_1_gir_LIBS     = libnimf.la
Nimf_0_1_gir_FILES    = $(introspection_sources)

INTROSPECTION_GIRS += Nimf-0.1.gir

girdir   = $(INTROSPECTION_GIRDIR)
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir   = $(INTROSPECTION_TYPELIBDIR)
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

uninstall-hook:
	-rmdir -p $(DESTDIR)$(libdir)/nimf
	-rmdir -p $(DESTDIR)$(IMdkitincludedir)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(gir_DATA) $(typelib_DATA)

DISTCLEANFILES = Makefile.in
