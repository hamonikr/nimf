project('nimf', 
    'c', 'cpp',
    version : '2019.08.14',
    license : 'LGPL3',
    meson_version : '>= 0.49.0',
    default_options : ['warning_level=1'])

# add_project_arguments('-Werror', language: 'c')
if get_option('enable_hardening').disabled()
    add_project_arguments('-D_FORTIFY_SOURCE=2', language : 'c')
endif

header_conf = configuration_data()
header_conf.set_quoted('VERSION', meson.project_version())
header_conf.set_quoted('PACKAGE', meson.project_name())

## I18n
add_project_arguments('-DGETTEXT_PACKAGE="' + meson.project_name() + '"',
    language : 'c')
subdir('po')
header_conf.set_quoted('NIMF_LOCALE_DIR', get_option('localedir'))

configure_file(output : 'config.h', configuration : header_conf)

conf_inc = include_directories('.')
libnimf_inc = include_directories('./libnimf')

subdir('libnimf')
subdir('modules')
subdir('bin')
subdir('data')
subdir('docs')
