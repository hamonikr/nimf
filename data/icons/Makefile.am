icon_themedir = $(datadir)/icons/hicolor

status_scalable_icondir   = $(icon_themedir)/scalable/status/
status_scalable_icon_DATA = \
	scalable/dasom-indicator.svg \
	scalable/dasom-indicator-warning.svg \
	scalable/dasom-indicator-en.svg \
	scalable/dasom-indicator-ko.svg

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

clean-local:
	-rmdir 22x22 24x24 32x32

uninstall-hook:
	 rm    -f $(DESTDIR)$(icon_themedir)/icon-theme.cache
	-rmdir -p $(DESTDIR)$(icon_themedir)/22x22/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/24x24/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/32x32/status
	-rmdir -p $(DESTDIR)$(icon_themedir)/scalable/status
	-$(GTK_UPDATE_ICON_CACHE) -f -t $(icon_themedir)

CLEANFILES = $(status_22_icon_DATA) \
	           $(status_24_icon_DATA) \
	           $(status_32_icon_DATA)

DISTCLEANFILES = Makefile.in
