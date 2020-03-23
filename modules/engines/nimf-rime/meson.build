deps = [common_deps, dependency('rime', version : '>= 1.2.9')]

sources = 'nimf-rime.c'

inc = include_directories('../../../libnimf')

c_args = ['-DG_LOG_DOMAIN="nimf"']

nimf_rime = shared_library('nimf-rime', 
    sources : sources,
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install: true,
    install_dir : module_dir)

install_data('org.nimf.engines.nimf-rime.gschema.xml',
	install_dir: get_option('datadir') + '/glib-2.0/schemas')
meson.add_install_script(meson.source_root() + '/meson_post_install.py')
