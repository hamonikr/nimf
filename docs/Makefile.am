DOC_MODULE=libnimf

DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.xml

DOC_SOURCE_DIR=$(top_srcdir)/libnimf

MKDB_OPTIONS=--xml-mode --output-format=xml

HFILE_GLOB=$(top_srcdir)/libnimf/*.h

CFILE_GLOB=$(top_srcdir)/libnimf/*.c

GTKDOC_LIBS= $(top_builddir)/libnimf/libnimf.la

include $(top_srcdir)/gtk-doc.make

DISTCLEANFILES = Makefile.in