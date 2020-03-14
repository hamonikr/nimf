deps = [dependency('glib-2.0'), dependency('gtk+-3.0')]

libdir = get_option('libdir')
c_args = ['-DG_LOG_DOMAIN="nimf"']
inc = [libnimf_inc, conf_inc]

shared_library('nimf-candidate',
    'nimf-candidate.c',
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true,
    install_dir : service_module_dir)
