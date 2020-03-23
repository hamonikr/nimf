m17n_db = dependency('m17n-db', version : '>= 1.7.0')

deps = [common_deps, 
    dependency('m17n-shell', version : '>= 1.7.0'), 
    m17n_db]

sources = 'nimf-m17n.c'

inc = include_directories('../../../libnimf')

c_args = ['-DG_LOG_DOMAIN="nimf"']
version = m17n_db.version().split('.')
C = version[0]
R = version[1]
M17N_1_8_0_AVAILABLE = C.to_int() > 1 or C.to_int() == 1 and R.to_int() >= 8
if M17N_1_8_0_AVAILABLE
    c_args += '-DM17N_1_8_0_AVAILABLE'
endif

nimf_m17n = shared_library('nimf-m17n.c', 
    sources : sources,
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true,
    install_dir : module_dir)

langs = ['am', 'ar', 'as', 'ath', 'be',  'bn', 'bo', 'cmc', 'cr', 'cs', 'da', 'dv', 'el', 'eo', 'fa', 'fr', 'grc', 'gu', 'he', 'hr', 'hy', 'ii', 'iu', 'ka', 'kk', 'km', 'ks', 'lo', 'mai', 'my', 'nsk', 'oj', 'or', 'pa', 'ps', 'ru', 'sa', 'sd', 'si', 'sk', 'sr', 'sv', 't', 'ta', 'tai', 'te', 'th', 'ug', 'uk', 'ur', 'uz', 'vi', 'yi']

link_with = [libnimf, nimf_m17n]

foreach lang : langs
    sources = ['nimf-m17n-@0@.c'.format(lang), 'nimf-m17n.h']
    shared_library('nimf-m17n-@0@'.format(lang),
        sources: sources, 
        dependencies: deps, 
        include_directories: inc, 
        c_args: c_args, 
        link_with : link_with,
        install : true,
        install_dir : module_dir)
endforeach

langs_with_gsettings = ['hi', 'hu', 'kn', 'ml', 'mr', 'ne']

install_data('org.nimf.engines.nimf-m17n.gschema.xml',
	install_dir: get_option('datadir') + '/glib-2.0/schemas')
meson.add_install_script(meson.source_root() + '/meson_post_install.py')

foreach lang : langs_with_gsettings
    install_data('org.nimf.engines.nimf-m17n-@0@.gschema.xml'.format(lang),
        install_dir: get_option('datadir') + '/glib-2.0/schemas')
    meson.add_install_script(meson.source_root() + '/meson_post_install.py')
endforeach