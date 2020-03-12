deps = [common_deps, dependency('x11')]

sources = [
    'nimf-xim.c',
    'nimf-xim.h',
    'nimf-xim-ic.c',
    'nimf-xim-ic.h',
    'IMdkit/FrameMgr.h',
    'IMdkit/Xi18n.h',
    'IMdkit/XimFunc.h',
    'IMdkit/XimProto.h',
    'IMdkit/FrameMgr.c',
    'IMdkit/i18nAttr.c',
    'IMdkit/i18nClbk.c',
    'IMdkit/i18nIc.c',
    'IMdkit/i18nIMProto.c',
    'IMdkit/i18nMethod.c',
    'IMdkit/i18nMethod.h',
    'IMdkit/i18nOffsetCache.c',
    'IMdkit/i18nPtHdr.c',
    'IMdkit/i18nUtil.c',
    'IMdkit/i18nX.c',
    'IMdkit/i18nX.h',
]

c_args = ['-DG_LOG_DOMAIN="nimf"']
inc = [libnimf_inc, conf_inc]

shared_library('nimf-xim',
    sources : sources,
    include_directories : inc,
    dependencies : deps,
    link_with : libnimf,
    c_args : c_args,
    install : true)