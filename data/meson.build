datadir = get_option('datadir')

install_data('nimf.conf', install_dir: '/etc/input.d')

rsvg_convert = find_program('rsvg-convert', 'rsvg')
assert(rsvg_convert.found(),
    'rsvg-convert or rsvg is not found, please install librsvg2-bin')

gtk_update_icon_cache = find_program('gtk-update-icon-cache-3.0', 'gtk-update-icon-cache', 
    required : true)
assert(gtk_update_icon_cache.found(),
    'gtk-update-icon-cache-3.0 or gtk-update-icon-cache not found')

icons_data = ['icons/scalable/nimf-logo.svg',
	'icons/scalable/nimf-focus-out.svg',
	'icons/scalable/nimf-system-keyboard.svg',
	'icons/scalable/nimf-libhangul.svg',
	'icons/scalable/nimf-anthy.svg',
	'icons/scalable/nimf-rime-simplified.svg',
	'icons/scalable/nimf-rime-traditional.svg']

icon_theme_dir = '@0@/icons/hicolor'.format(datadir)

install_data(sources : icons_data, 
    install_dir : icon_theme_dir + '/scalable/status')

foreach p : ['16', '22', '24', '32', '48', '96', '256']
    foreach icon : icons_data
        basename = icon.split('/')[2].split('.')[0]
        scaled_icon = '@0@-@1@.png'.format(p, basename)
        run_command(rsvg_convert, '-w', p, '-h', p, '-o', scaled_icon, icon)
        install_data(scaled_icon, rename : basename + '.png',
            install_dir : icon_theme_dir + '/@0@x@1@/status'.format(p, p))
    endforeach
endforeach

install_data('apparmor-abstractions/nimf', install_dir : '/etc/apparmor.d/abstractions')

if get_option('with_im_config_data')
    install_data(sources : ['im-config/23_nimf.conf', 'im-config/23_nimf.rc'],
        install_dir : '/usr/share/im-config/data')
endif

if get_option('with_imsettings_data')
    install_data('nimf.conf', install_dir : '/etc/X11/xinit/xinput.d')
endif
