gnome = import('gnome')

deps = [dependency('glib-2.0'),
    dependency('gobject-2.0'),
    declare_dependency(link_with : libnimf)]

gnome.gtkdoc('nimf',
    main_sgml : 'nimf-docs.xml',
    src_dir : ['@0@/libnimf'.format(meson.source_root()), '@0@/bin'.format(meson.source_root()) ],
    mkdb_args : ['--xml-mode', '--output-format=xml', '--name-space=nimf'],
    scan_args : ['--rebuild-types'],
    ignore_headers: ['nimf-enum-types-private.h',
        'nimf-marshalers-private.h',
        'nimf-message-enum-types-private.h',
        'nimf-message-private.h',
        'nimf-module-private.h',
        'nimf-server-private.h'],
    dependencies : deps,
    install : true)
