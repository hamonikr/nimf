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

gsettings_SCHEMAS = org.nimf.clients.qt5.gschema.xml
@GSETTINGS_RULES@

qt5_im_moduledir = $(QT5_IM_MODULE_DIR)
qt5_im_module_LTLIBRARIES = libqt5im-nimf.la

BUILT_SOURCES = im-nimf-qt5.moc
libqt5im_nimf_la_SOURCES = im-nimf-qt5.cpp $(BUILT_SOURCES)

libqt5im_nimf_la_CXXFLAGS = \
	$(EXTRA_CFLAGS) \
	$(LIBNIMF_DEPS_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-I$(top_srcdir)/libnimf/client \
	-I$(top_srcdir)/libnimf/server \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_COMPILATION \
	$(NIMF_QT5_DEPS_CFLAGS) \
	-I$(QT5_CORE_PRIVATE_INCLUDE_PATH) \
	-I$(QT5_GUI_PRIVATE_INCLUDE_PATH) \
	-DQT_NO_KEYWORDS \
	-fPIC

libqt5im_nimf_la_LDFLAGS = -avoid-version -module \
	                         $(NIMF_QT5_DEPS_LIBS) \
	                         $(LIBNIMF_DEPS_LIBS)
libqt5im_nimf_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

im-nimf-qt5.moc: im-nimf-qt5.cpp
	$(AM_V_GEN) $(QT5_MOC) -I $(QT5_CORE_PRIVATE_INCLUDE_PATH) \
	                       -I $(QT5_GUI_PRIVATE_INCLUDE_PATH) im-nimf-qt5.cpp \
	                       -o im-nimf-qt5.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.so
	rm -f    $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.so
	-rmdir -p $(DESTDIR)$(qt5_im_moduledir)

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
