qt5 = import('qt5')
qt5_deps = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'])
qt5_core = dependency('Qt5Core')
qt5_version = qt5_core.version()
qt5_inc = qt5_core.get_pkgconfig_variable('includedir')
qt5_libdir = qt5_core.get_pkgconfig_variable('libdir')

## meson fs module is available on version >= 0.53.0
# fs = import('fs')
# if fs.is_dir(qt5_libdir + '/qt5')
if run_command('[', '-d', qt5_libdir + '/qt5', ']').returncode() == 0
    qt5_im_module_dir = qt5_libdir + '/qt5/plugins/platforminputcontexts'
# elif fs.is_dir(qt5_libdir + '/qt')
elif run_command('[', '-d', qt5_libdir + '/qt', ']').returncode() == 0
    qt5_im_module_dir = qt5_libdir + '/qt/plugins/platforminputcontexts'
else
    error('Can not determine Qt5 plugins/platforminputcontexts directory')
endif

qt5_core_private_inc = qt5_inc + '/QtCore/' + qt5_version
qt5_gui_private_inc = qt5_inc + '/QtGui/' + qt5_version
inc = [libnimf_inc,
    include_directories(qt5_core_private_inc, qt5_gui_private_inc)]

moc_files = qt5.preprocess(moc_sources : 'im-nimf-qt5.cpp',
    moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'],
    include_directories: include_directories(qt5_core_private_inc, qt5_gui_private_inc))

cpp_args = [
    '-DG_LOG_DOMAIN="nimf"',
    '-DNIMF_COMPILATION',
    '-DQT_NO_KEYWORDS',
    '-fPIC'
]

deps = [qt5_deps,
    dependency('glib-2.0'),
    dependency('gobject-2.0'),
    dependency('gio-2.0')]

shared_library('qt5im_nimf', 
    sources : ['im-nimf-qt5.cpp', moc_files],
    include_directories: inc,
    cpp_args : cpp_args,
    cpp_std : 'c++11',
    link_with: libnimf,
    dependencies : deps,
    install : true,
    install_dir : qt5_im_module_dir)

install_data('org.nimf.clients.qt5.gschema.xml', 
    install_dir : get_option('datadir') + '/glib-2.0/schemas')
meson.add_install_script(meson.source_root() + '/meson_post_install.py')
