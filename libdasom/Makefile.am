lib_LTLIBRARIES = libdasom.la

BUILT_SOURCES =      \
	dasom-enum-types.h \
	dasom-enum-types.c \
	dasom-marshalers.h \
	dasom-marshalers.c \
	$(NULL)

IMdkit_SOURCES = \
	IMdkit/FrameMgr.h \
	IMdkit/IMdkit.h \
	IMdkit/Xi18n.h \
	IMdkit/Xi18nX.h \
	IMdkit/XimFunc.h \
	IMdkit/XimProto.h \
	IMdkit/Xtrans.h \
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

libdasom_la_SOURCES = \
	dasom.h \
	dasom-private.h \
	dasom-private.c \
	dasom-im.c \
	dasom-im.h \
	dasom-types.c \
	dasom-types.h \
	dasom-events.c \
	dasom-events.h \
	dasom-engine.c \
	dasom-engine.h \
	dasom-module.h \
	dasom-module.c \
	dasom-module-manager.h \
	dasom-module-manager.c \
	dasom-key-syms.h \
	dasom-message.h \
	dasom-message.c \
	dasom-candidate.h \
	dasom-candidate.c \
	dasom-agent.c \
	dasom-agent.h \
	dasom-connection.c \
	dasom-connection.h \
	dasom-server.c \
	dasom-server.h \
	$(BUILT_SOURCES) \
	$(IMdkit_SOURCES) \
	$(NULL)

libdasom_la_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libdasom \
	-DDASOM_COMPILATION \
	-DG_LOG_DOMAIN=\"dasom\" \
	-DDASOM_MODULE_DIR=\"$(libdir)/dasom/modules\" \
	$(LIBDASOM_DEPS_CFLAGS)

# TODO: libdasom 에 대한 header 파일과 doc을 배포하기 전까지 libdasom.so 에
# 버전을 붙이지 않습니다.
libdasom_la_LDFLAGS = $(LIBDASOM_DEPS_LIBS) -avoid-version

dasomincludedir = $(includedir)/dasom
dasominclude_HEADERS =

dasom-marshalers.h: dasom-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=dasom_cclosure_marshal \
	                            --header dasom-marshalers.list > dasom-marshalers.h

dasom-marshalers.c: dasom-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=dasom_cclosure_marshal \
	                            --body dasom-marshalers.list > dasom-marshalers.c

dasom-enum-types.h: dasom-message.h dasom-enum-types.h.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Dasom \
	                         --template dasom-enum-types.h.template \
	                           dasom-message.h dasom-key-syms.h dasom-types.h > dasom-enum-types.h

dasom-enum-types.c: dasom-message.h dasom-enum-types.c.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Dasom \
	                         --template dasom-enum-types.c.template \
	                           dasom-message.h dasom-key-syms.h dasom-types.h > dasom-enum-types.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dasom.pc

gsettings_SCHEMAS = org.freedesktop.Dasom.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = dasom.pc.in

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS  = --add-include-path=$(srcdir) \
                              --warn-all --warn-error \
                              --identifier-prefix=Dasom --symbol-prefix=dasom
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = dasom-agent.h

Dasom-0.1.gir: libdasom.la
Dasom_0_1_gir_INCLUDES = GObject-2.0
Dasom_0_1_gir_CFLAGS   = $(libdasom_la_CFLAGS) -I$(top_srcdir)
Dasom_0_1_gir_LIBS     = libdasom.la
Dasom_0_1_gir_FILES    = $(introspection_sources)

INTROSPECTION_GIRS += Dasom-0.1.gir

girdir   = $(INTROSPECTION_GIRDIR)
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir   = $(INTROSPECTION_TYPELIBDIR)
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

uninstall-hook:
	-rmdir $(DESTDIR)$(libdir)/dasom

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(gir_DATA) $(typelib_DATA)

DISTCLEANFILES = Makefile.in
