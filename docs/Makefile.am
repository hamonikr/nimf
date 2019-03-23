DOC_MODULE = nimf

DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.xml

DOC_SOURCE_DIR = \
	$(top_srcdir)/libnimf \
	$(top_srcdir)/bin

MKDB_OPTIONS = --xml-mode --output-format=xml --name-space=nimf

SCAN_OPTIONS = --rebuild-types

HFILE_GLOB = \
	$(top_srcdir)/libnimf/*.h \
	$(top_builddir)/libnimf/*.h

CFILE_GLOB = \
	$(top_srcdir)/libnimf/*.c \
	$(top_srcdir)/bin/nimf/nimf.c \
	$(top_srcdir)/bin/nimf-settings/nimf-settings.c

IGNORE_HFILES = \
	nimf-enum-types-private.h \
	nimf-marshalers-private.h \
	nimf-message-enum-types-private.h \
	nimf-message-private.h \
	nimf-module-private.h \
	nimf-server-private.h

GTKDOC_LIBS = $(top_builddir)/libnimf/libnimf.la

include $(top_srcdir)/gtk-doc.make

DISTCLEANFILES = Makefile.in
