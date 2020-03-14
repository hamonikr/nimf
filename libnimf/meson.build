gnome = import('gnome')

nimf_marshalers = gnome.genmarshal('nimf-marshalers', 
    sources: 'nimf-marshalers.list',
prefix : 'nimf_cclosure_marshal')
nimf_marshalers_c = nimf_marshalers[0]
nimf_marshalers_h = nimf_marshalers[1]

nimf_marshalers_private_h = gnome.genmarshal('nimf-marshalers-private', 
    sources: 'nimf-marshalers.list',
prefix : 'nimf_cclosure_marshal')[1]

enum_types = gnome.mkenums('nimf-enum-types',
    identifier_prefix : 'Nimf',
    c_template : 'nimf-enum-types.c.template',
    h_template : 'nimf-enum-types-private.h.template',
    sources : ['nimf-key-syms.h', 'nimf-types.h']
)
enum_types_c = enum_types[0]
enum_types_private_h = enum_types[1]

message_enum_types = gnome.mkenums('nimf-message-enum-types',
    identifier_prefix : 'Nimf',
    c_template : 'nimf-message-enum-types.c.template',
    h_template : 'nimf-message-enum-types-private.h.template',
    sources : ['nimf-message-private.h']
)
message_enum_types_c = message_enum_types[0]
message_enum_types_private_h = message_enum_types[1]

built_sources = [
    nimf_marshalers_c,
	nimf_marshalers_h,
    nimf_marshalers_private_h,
    enum_types_c,
	enum_types_private_h,
    message_enum_types_c,
	message_enum_types_private_h
]

sources = [
	'nimf.h',
	'nimf-candidatable.c',
	'nimf-candidatable.h',
	'nimf-engine.c',
	'nimf-engine.h',
	'nimf-events.c',
	'nimf-events.h',
	'nimf-im.c',
	'nimf-im.h',
	'nimf-key-syms.h',
	'nimf-message.c',
	'nimf-message-private.h',
	'nimf-module.c',
	'nimf-module-private.h',
	'nimf-preeditable.c',
	'nimf-preeditable.h',
	'nimf-server.c',
	'nimf-server.h',
	'nimf-server-private.h',
	'nimf-service.c',
	'nimf-service.h',
	'nimf-service-ic.c',
	'nimf-service-ic.h',
	'nimf-service-ic-private.h',
	'nimf-types.c',
	'nimf-types.h',
	'nimf-utils.c',
	'nimf-utils.h',
	'nimf-utils-private.h',
    built_sources
]

deps = [
	dependency('glib-2.0'),
	dependency('gio-2.0'),
	dependency('gio-unix-2.0'),
	dependency('gobject-2.0'),
	dependency('gmodule-2.0')
]

libdir = get_option('libdir')

inc = [conf_inc, 
	include_directories('.')]

c_args = ['-DNIMF_COMPILATION',
	'-DG_LOG_DOMAIN="nimf"',
	'-DNIMF_MODULE_DIR="@0@/nimf/modules"'.format(libdir),
	'-DNIMF_SERVICE_MODULE_DIR="@0@/nimf/modules/services"'.format(libdir)]

libnimf = shared_library('nimf', sources,
    dependencies: deps,
	c_args : c_args,
	# link_args : ['-version-info=1:0:0'],
    include_directories : inc,
	install : true)

install_data('org.nimf.gschema.xml', 
	install_dir: get_option('datadir') / 'glib-2.0' / 'schemas')
meson.add_install_script(meson.source_root() / 'meson_post_install.py')
