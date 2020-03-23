project('nimf', 
    'c', 'cpp',
    version : '2019.08.14',
    license : 'LGPL3',
    meson_version : '>= 0.45.1',
    default_options : ['warning_level=1'])

# add_project_arguments('-Werror', language: 'c')
if not get_option('enable_hardening')
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

libdir = get_option('libdir')
prefix = get_option('prefix')
service_module_dir = '@0@/nimf/modules/services'.format(libdir)
module_dir = '@0@/nimf/modules'.format(libdir)
header_conf.set_quoted('NIMF_MODULE_DIR', '@0@/@1@'.format(prefix, module_dir))
header_conf.set_quoted('NIMF_SERVICE_MODULE_DIR', '@0@/@1@'.format(prefix, service_module_dir))

configure_file(output : 'config.h', configuration : header_conf)

conf_inc = include_directories('.')
libnimf_inc = include_directories('./libnimf')

subdir('libnimf')
subdir('modules')
subdir('bin')
subdir('data')
subdir('docs')
