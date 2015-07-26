bin_PROGRAMS = dasom-agent

dasom_agent_SOURCES = main.c

dasom_agent_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libdasom \
	-DDASOM_COMPILATION \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(X11_CFLAGS) \
	$(GTK3_CFLAGS)

dasom_agent_LDADD = \
	$(top_builddir)/libdasom/libdasom.la \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X11_LIBS) \
	$(GTK3_LIBS)
