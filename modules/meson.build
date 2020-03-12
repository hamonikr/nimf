common_deps = [dependency('glib-2.0'),
    dependency('gobject-2.0'),
    dependency('gio-2.0')]

subdir('engines/nimf-system-keyboard')
subdir('engines/nimf-libhangul')
if get_option('with_nimf_anthy')
    subdir('engines/nimf-anthy')
endif

if get_option('with_nimf_rime')
    subdir('engines/nimf-rime')
endif

if get_option('with_nimf_m17n')
    subdir('engines/nimf-m17n')
endif

subdir('clients/gtk')
subdir('clients/qt5')
if dependency('qt4', modules: ['Core', 'Gui', 'Widgets'], required : false).found()
    subdir('clients/qt4')
endif

subdir('services/candidate')
subdir('services/indicator')
subdir('services/nim')
subdir('services/preedit-window')
subdir('services/wayland')
subdir('services/xim')
