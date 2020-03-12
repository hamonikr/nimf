deps = [dependency('glib-2.0'), dependency('gtk+-3.0')]

c_args = ['-DG_LOG_DOMAIN="nimf"']
inc = [libnimf_inc, conf_inc]

shared_library('nimf-nim',
    sources : 'nimf-preedit-window.c',
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true)
