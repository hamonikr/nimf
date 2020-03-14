libdir = get_option('libdir')
datadir = get_option('datadir')

deps = [dependency('glib-2.0'), dependency('gio-2.0')]
inc = [conf_inc, include_directories('../libnimf')]
c_args = ['-DNIMF_COMPILATION']

executable('nimf', 'nimf/nimf.c', 
    dependencies : deps,
    c_args : c_args,
    include_directories : inc,
    link_with : libnimf,
    install : true)

settings_c_args = ['-DNIMF_COMPILATION']

settings_deps = [dependency('gtk+-3.0'), 
    dependency('gmodule-2.0'), 
    dependency('libxklavier')]

executable('nimf-settings', 
    sources : ['nimf-settings/nimf-settings.c', built_sources],
    dependencies : settings_deps,
    c_args : settings_c_args,
    include_directories : inc,
    link_with : libnimf, 
    install : true)

i18n = import('i18n')
i18n.merge_file(
  input: 'nimf-settings/nimf-settings.desktop.in',
  output: 'nimf-settings.desktop',
  type: 'desktop',
  po_dir: '../../po',
  install: true,
  install_dir: get_option('datadir') / 'applications'
)
i18n.merge_file(
  input: 'nimf-settings/nimf-settings-autostart.desktop.in',
  output: 'nimf-settings-autostart.desktop',
  type: 'desktop',
  po_dir: '../../po',
  install: true,
  install_dir: '/etc/xdg/autostart'
)

install_data('nimf-settings/org.nimf.settings.gschema.xml', 
    install_dir : get_option('datadir') / 'glib-2.0' / 'schemas')
meson.add_install_script(meson.source_root() / 'meson_post_install.py')

install_man('nimf/nimf.1')