deps = [common_deps, dependency('gio-unix-2.0')]

libdir = get_option('libdir')
c_args = ['-DG_LOG_DOMAIN="nimf"']
inc = [libnimf_inc, conf_inc]

sources = ['nimf-nim.c',
    'nimf-nim.h',
    'nimf-nim-ic.c',
    'nimf-nim-ic.h',
    'nimf-connection.c',
    'nimf-connection.h']

shared_library('nimf-nim',
    sources : sources,
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true)
