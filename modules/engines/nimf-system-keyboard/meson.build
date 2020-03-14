deps = [common_deps, dependency('xkbcommon', version : '>= 0.5.0')]

sources = 'nimf-system-keyboard.c'

inc = include_directories('../../../libnimf')

c_args = ['-DG_LOG_DOMAIN="nimf"']

nimf_system_keyboard = shared_library('nimf-system-keyboard', 
    sources : sources,
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install: true,
    install_dir : module_dir)

install_data('org.nimf.engines.nimf-system-keyboard.gschema.xml',
    install_dir: get_option('datadir') / 'glib-2.0' / 'schemas')
meson.add_install_script(meson.source_root() / 'meson_post_install.py')
