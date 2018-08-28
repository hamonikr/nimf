#
# # 법적 고지
#
# Nimf 소프트웨어는 대한민국 저작권법과 국제 조약의 보호를 받습니다.
# Nimf 개발자는 대한민국 법률의 보호를 받습니다.
# 커뮤니티의 위력을 이용하여 개발자의 시간과 노동력을 약탈하려는 행위를 금하시기 바랍니다.
#
# * 커뮤니티 게시판에 개발자를 욕(비난)하거나
# * 욕보이는(음해하는) 글을 작성하거나
# * 허위 사실을 공표하거나
# * 명예를 훼손하는
#
# 등의 행위는 정보통신망 이용촉진 및 정보보호 등에 관한 법률의 제재를 받습니다.
#
# # 면책 조항
#
# Nimf 는 무료로 배포되는 오픈소스 소프트웨어입니다.
# Nimf 개발자는 개발 및 유지보수에 대해 어떠한 의무도 없고 어떠한 책임도 없습니다.
# 어떠한 경우에도 보증하지 않습니다. 도덕적 보증 책임도 없고, 도의적 보증 책임도 없습니다.
# Nimf 개발자는 리브레오피스, 이클립스 등 귀하가 사용하시는 소프트웨어의 버그를 해결해야 할 의무가 없습니다.
# Nimf 개발자는 귀하가 사용하시는 배포판에 대해 기술 지원을 해드려야 할 의무가 없습니다.
#

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
