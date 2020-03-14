deps =[dependency('glib-2.0'), 
    dependency('x11'), 
    dependency('gtk+-3.0'), 
    dependency('appindicator3-0.1'), 
    dependency('libxklavier'),
    dependency('gdk-3.0'), 
    dependency('gdk-x11-3.0'), 
    dependency('gmodule-2.0')]

sources = ['nimf-indicator.c']
c_args = ['-DG_LOG_DOMAIN="nimf"']
inc = [libnimf_inc, conf_inc]

shared_library('nimf-indicator', 
    sources : sources,
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true,
    install_dir : service_module_dir)