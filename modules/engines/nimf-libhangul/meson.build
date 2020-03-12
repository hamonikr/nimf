deps = [common_deps, dependency('libhangul', version : '>= 0.0.12')]

sources = 'nimf-libhangul.c'

inc = include_directories('../../../libnimf')

libdir = get_option('libdir')
c_args = ['-DG_LOG_DOMAIN="nimf"',
    '-DMSSYMBOL_PATH="@0@/@1@"'.format(libdir + '/nimf', 'mssymbol.txt')]

nimf_libhangul = shared_library('nimf-libhangul', 
    sources : sources,
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install: true)

install_data('org.nimf.engines.nimf-libhangul.gschema.xml', 
    install_dir: get_option('datadir') / 'glib-2.0' / 'schemas')
meson.add_install_script(meson.source_root() / 'meson_post_install.py')
