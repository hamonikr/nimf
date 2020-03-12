libdir = get_option('libdir')
datadir = get_option('datadir')

deps = [dependency('glib-2.0'), dependency('gio-2.0')]
inc = [conf_inc, include_directories('../libnimf')]
c_args = ['-DNIMF_COMPILATION']

executable('nimf', 'nimf/nimf.c', 
    dependencies : deps,
    c_args: c_args,
    include_directories : inc,
    link_with : libnimf)

settings_c_args = ['-DNIMF_COMPILATION',
    '-DNIMF_MODULE_DIR="@0@/nimf/modules"'.format(libdir)]

settings_deps = [dependency('gtk+-3.0'), 
    dependency('gmodule-2.0'), 
    dependency('libxklavier')]

executable('nimf-settings', 
    sources : ['nimf-settings/nimf-settings.c', built_sources],
    dependencies : settings_deps,
    c_args : settings_c_args,
    include_directories : inc,
    link_with : libnimf)

install_data('nimf-settings/org.nimf.settings.gschema.xml', 
    install_dir : get_option('datadir') / 'glib-2.0' / 'schemas')
meson.add_install_script(meson.source_root() / 'meson_post_install.py')

install_man('nimf/nimf.1')