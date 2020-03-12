wayland_protocols = dependency('wayland-protocols')

deps = [common_deps, 
    dependency('wayland-client'),
    dependency('wayland-server'),
    wayland_protocols,
    dependency('xkbcommon')]

wayland_im_xml_path = wayland_protocols.get_pkgconfig_variable('pkgdatadir') \
    + '/unstable/input-method/input-method-unstable-v1.xml'

wayland_scanner = find_program('wayland-scanner')

input_method_unstable_v1_client_protocol_h = custom_target('input-method-unstable-v1-client-protocol.h',
    output : 'input-method-unstable-v1-client-protocol.h',
    input : wayland_im_xml_path,
    command : [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'])

input_method_unstable_v1_protocol_c = custom_target('input-method-unstable-v1-protocol.c',
    output : 'input-method-unstable-v1-protocol.c',
    input : wayland_im_xml_path,
    command : [wayland_scanner, 'code', '@INPUT@', '@OUTPUT@'])

sources = ['nimf-wayland.c',
    'nimf-wayland-ic.c',
    'nimf-wayland-ic.h',
    input_method_unstable_v1_client_protocol_h,
    input_method_unstable_v1_protocol_c]

inc = [libnimf_inc, conf_inc]

c_args = ['-DG_LOG_DOMAIN="nimf"']

shared_library('nimf-wayland',
    sources : sources,
    include_directories : inc,
    dependencies : deps,
    link_with : libnimf,
    c_args : c_args,
    install : true)