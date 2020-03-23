deps = [common_deps, dependency('anthy')]

## meson fs module is available on version >= 0.53.0
# fs = import('fs')
# is_anthy_dic_exists = fs.is_file('/var/lib/anthy/anthy.dic') or
    # fs.is_file('/usr/share/anthy/anthy.dic')

is_var_lib_anthy_dic_exists = run_command('[', '-e', '/var/lib/anthy/anthy.dic', ']').returncode() == 0
is_usr_anthy_dic_exists = run_command('[', '-e', '/usr/share/anthy/anthy.dic', ']').returncode() == 0
is_anthy_dic_exists = is_var_lib_anthy_dic_exists or is_usr_anthy_dic_exists

assert(is_anthy_dic_exists, 'anthy.dic is not found.')

sources = 'nimf-anthy.c'

inc = include_directories('../../../libnimf')

c_args = ['-DG_LOG_DOMAIN="nimf"']

nimf_anthy = shared_library('nimf-anthy', 
    sources : sources,
    dependencies : deps,
    include_directories : inc,
    c_args : c_args,
    link_with : libnimf,
    install : true,
    install_dir : module_dir)

install_data('org.nimf.engines.nimf-anthy.gschema.xml',
    install_dir : get_option('datadir') + '/glib-2.0/schemas')
meson.add_install_script(meson.source_root() + '/meson_post_install.py')
