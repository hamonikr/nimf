inc = [libnimf_inc, conf_inc]
c_args = ['-DG_LOG_DOMAIN="nimf"', '-Wno-deprecated-declarations']

gtk3 = dependency('gtk+-3.0')

gtk3_libdir = gtk3.get_pkgconfig_variable('libdir')
gtk3_binary_version = gtk3.get_pkgconfig_variable('gtk_binary_version')
gtk3_im_module_dir = '@0@/gtk-3.0/@1@/immodules'.format(gtk3_libdir, gtk3_binary_version)

gtk3_query_immodules = find_program('gtk-query-immodules-3.0')
assert(gtk3_query_immodules.found(), 'gtk-query-immodules-3.0 not found')

shared_library('im-nimf-gtk3',
    name_prefix : '',
    sources : 'im-nimf.c',
    dependencies : [gtk3, dependency('glib-2.0')],
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true,
    install_dir : gtk3_im_module_dir)

gtk2 = dependency('gtk+-2.0')
gtk2_libdir = gtk2.get_pkgconfig_variable('libdir')
gtk2_binary_version = gtk2.get_pkgconfig_variable('gtk_binary_version')
gtk2_im_module_dir = '@0@/gtk-2.0/@1@/immodules'.format(gtk2_libdir, gtk2_binary_version)

gtk2_query_immodules = find_program('gtk-query-immodules-2.0')
assert(gtk2_query_immodules.found(), 'gtk-query-immodules-2.0 not found')

shared_library('im-nimf-gtk2',
    name_prefix : '',
    sources : 'im-nimf.c',
    dependencies : [gtk2, dependency('glib-2.0')],
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true,
    install_dir : gtk2_im_module_dir)


install_data('org.nimf.clients.gtk.gschema.xml', 
    install_dir : get_option('datadir') + '/glib-2.0/schemas')
meson.add_install_script(meson.source_root() + '/meson_post_install.py')
