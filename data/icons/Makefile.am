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

icon_themedir = $(datadir)/icons/hicolor

status_scalable_icondir   = $(icon_themedir)/scalable/status/
status_scalable_icon_DATA = \
	scalable/nimf-logo.svg \
	scalable/nimf-focus-out.svg \
	scalable/nimf-system-keyboard.svg \
	scalable/nimf-libhangul.svg \
	scalable/nimf-anthy.svg \
	scalable/nimf-chewing.svg \
	scalable/nimf-rime-simplified.svg \
	scalable/nimf-rime-traditional.svg

# 16x16 icons

status_16_icondir   = $(icon_themedir)/16x16/status/
status_16_icon_tmp  = $(status_scalable_icon_DATA:scalable/%=16x16/%)
status_16_icon_DATA = $(status_16_icon_tmp:.svg=.png)

$(status_16_icon_DATA): $(status_scalable_icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) 16x16; \
	  $(RSVG_CONVERT) -w 16 -h 16 scalable/`basename "$@" .png`.svg -o $@

# 22x22 icons

status_22_icondir   = $(icon_themedir)/22x22/status/
status_22_icon_tmp  = $(status_scalable_icon_DATA:scalable/%=22x22/%)
status_22_icon_DATA = $(status_22_icon_tmp:.svg=.png)

$(status_22_icon_DATA): $(status_scalable_icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) 22x22; \
	  $(RSVG_CONVERT) -w 22 -h 22 scalable/`basename "$@" .png`.svg -o $@

# 24x24 icons

status_24_icondir   = $(icon_themedir)/24x24/status/
status_24_icon_tmp  = $(status_scalable_icon_DATA:scalable/%=24x24/%)
status_24_icon_DATA = $(status_24_icon_tmp:.svg=.png)

$(status_24_icon_DATA): $(status_scalable_icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) 24x24; \
	  $(RSVG_CONVERT) -w 24 -h 24 scalable/`basename "$@" .png`.svg -o $@

# 32x32 icons

status_32_icondir   = $(icon_themedir)/32x32/status/
status_32_icon_tmp  = $(status_scalable_icon_DATA:scalable/%=32x32/%)
status_32_icon_DATA = $(status_32_icon_tmp:.svg=.png)

$(status_32_icon_DATA): $(status_scalable_icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) 32x32; \
	  $(RSVG_CONVERT) -w 32 -h 32 scalable/`basename "$@" .png`.svg -o $@

update-gtk-icon-cache:
	@if [ -d "$(icon_themedir)" ]; then \
	  $(GTK_UPDATE_ICON_CACHE) -f -t $(icon_themedir); \
	else \
	  echo "GTK icon cache is not updated. Try after make; sudo make install"; \
	  exit 1; \
	fi

# 48x48 icons

status_48_icondir   = $(icon_themedir)/48x48/status/
status_48_icon_tmp  = $(status_scalable_icon_DATA:scalable/%=48x48/%)
status_48_icon_DATA = $(status_48_icon_tmp:.svg=.png)

$(status_48_icon_DATA): $(status_scalable_icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) 48x48; \
	  $(RSVG_CONVERT) -w 48 -h 48 scalable/`basename "$@" .png`.svg -o $@

# 96x96 icons

status_96_icondir   = $(icon_themedir)/96x96/status/
status_96_icon_tmp  = $(status_scalable_icon_DATA:scalable/%=96x96/%)
status_96_icon_DATA = $(status_96_icon_tmp:.svg=.png)

$(status_96_icon_DATA): $(status_scalable_icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) 96x96; \
	  $(RSVG_CONVERT) -w 96 -h 96 scalable/`basename "$@" .png`.svg -o $@

# 256x256 icons

status_256_icondir   = $(icon_themedir)/256x256/status/
status_256_icon_tmp  = $(status_scalable_icon_DATA:scalable/%=256x256/%)
status_256_icon_DATA = $(status_256_icon_tmp:.svg=.png)

$(status_256_icon_DATA): $(status_scalable_icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) 256x256; \
	  $(RSVG_CONVERT) -w 256 -h 256 scalable/`basename "$@" .png`.svg -o $@

clean-local:
	-rmdir 16x16 22x22 24x24 32x32 48x48 96x96 256x256

uninstall-hook:
	 rm    -f $(DESTDIR)$(icon_themedir)/icon-theme.cache
	-rmdir -p $(DESTDIR)$(icon_themedir)/16x16/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/22x22/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/24x24/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/32x32/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/48x48/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/96x96/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/256x256/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/scalable/status
	-$(GTK_UPDATE_ICON_CACHE) -f -t $(icon_themedir)

CLEANFILES = $(status_16_icon_DATA) \
	           $(status_22_icon_DATA) \
	           $(status_24_icon_DATA) \
	           $(status_32_icon_DATA) \
	           $(status_48_icon_DATA) \
	           $(status_96_icon_DATA) \
	           $(status_256_icon_DATA)

DISTCLEANFILES = Makefile.in
