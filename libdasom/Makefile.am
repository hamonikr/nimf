# 후보창 때문에 gtk3을 추가하는데 나중에 모듈로 분리하면서 제거해야 합니다.

SUBDIRS = IMdkit

lib_LTLIBRARIES = libdasom.la

BUILT_SOURCES =      \
	dasom-enum-types.h \
	dasom-enum-types.c \
	dasom-marshalers.h \
	dasom-marshalers.c \
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
	$(NULL)

libdasom_la_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libdasom \
	-DDASOM_COMPILATION \
	-DG_LOG_DOMAIN=\"dasom\" \
	-DDASOM_MODULE_DIR=\"$(libdir)/dasom/modules\" \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(GTK3_CFLAGS)

libdasom_la_LDFLAGS = $(GTK3_LIBS)
libdasom_la_LIBADD = IMdkit/libIMdkit.la

dasomincludedir = $(includedir)/dasom
dasominclude_HEADERS = \
	dasom.h \
	dasom-im.h \
	dasom-types.h \
	dasom-events.h \
	dasom-engine.h \
	dasom-key-syms.h

dasom-marshalers.h: dasom-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=dasom_cclosure_marshal --header dasom-marshalers.list > dasom-marshalers.h

dasom-marshalers.c: dasom-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=dasom_cclosure_marshal --body dasom-marshalers.list > dasom-marshalers.c

dasom-enum-types.h: dasom-message.h dasom-enum-types.h.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Dasom --template \
	    dasom-enum-types.h.template dasom-message.h dasom-key-syms.h dasom-types.h > dasom-enum-types.h

dasom-enum-types.c: dasom-message.h dasom-enum-types.c.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Dasom --template \
	    dasom-enum-types.c.template dasom-message.h dasom-key-syms.h dasom-types.h > dasom-enum-types.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dasom.pc

gsettings_SCHEMAS = org.freedesktop.Dasom.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = \
	dasom.pc.in

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS  = --add-include-path=$(srcdir) --warn-all \
                              --identifier-prefix=Dasom --symbol-prefix=dasom
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = $(libdasom_la_SOURCES)

Dasom-0.1.gir: libdasom.la
Dasom_0_1_gir_INCLUDES = GObject-2.0
Dasom_0_1_gir_CFLAGS = $(libdasom_la_CFLAGS) -I$(top_srcdir)
Dasom_0_1_gir_LIBS = libdasom.la
Dasom_0_1_gir_FILES = $(introspection_sources)

INTROSPECTION_GIRS += Dasom-0.1.gir

girdir = $(INTROSPECTION_GIRDIR)
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(INTROSPECTION_TYPELIBDIR)
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = \
	dasom-marshalers.h dasom-marshalers.c \
	dasom-enum-types.h dasom-enum-types.c \
	$(gir_DATA) $(typelib_DATA)
