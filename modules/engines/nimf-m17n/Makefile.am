SUBDIRS = icons

moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = \
	libnimf-m17n.la \
	libnimf-m17n-am.la \
	libnimf-m17n-ar.la \
	libnimf-m17n-as.la \
	libnimf-m17n-ath.la \
	libnimf-m17n-be.la \
	libnimf-m17n-bla.la \
	libnimf-m17n-bn.la \
	libnimf-m17n-bo.la \
	libnimf-m17n-cmc.la \
	libnimf-m17n-cr.la \
	libnimf-m17n-cs.la \
	libnimf-m17n-da.la \
	libnimf-m17n-dv.la \
	libnimf-m17n-el.la \
	libnimf-m17n-eo.la \
	libnimf-m17n-fa.la \
	libnimf-m17n-fr.la \
	libnimf-m17n-grc.la \
	libnimf-m17n-gu.la \
	libnimf-m17n-he.la \
	libnimf-m17n-hr.la \
	libnimf-m17n-hy.la \
	libnimf-m17n-ii.la \
	libnimf-m17n-iu.la \
	libnimf-m17n-ka.la \
	libnimf-m17n-kk.la \
	libnimf-m17n-km.la \
	libnimf-m17n-ks.la \
	libnimf-m17n-lo.la \
	libnimf-m17n-mai.la \
	libnimf-m17n-my.la \
	libnimf-m17n-nsk.la \
	libnimf-m17n-oj.la \
	libnimf-m17n-or.la \
	libnimf-m17n-pa.la \
	libnimf-m17n-ps.la \
	libnimf-m17n-ru.la \
	libnimf-m17n-sa.la \
	libnimf-m17n-sd.la \
	libnimf-m17n-si.la \
	libnimf-m17n-sk.la \
	libnimf-m17n-sr.la \
	libnimf-m17n-sv.la \
	libnimf-m17n-t.la \
	libnimf-m17n-ta.la \
	libnimf-m17n-tai.la \
	libnimf-m17n-te.la \
	libnimf-m17n-th.la \
	libnimf-m17n-ug.la \
	libnimf-m17n-uk.la \
	libnimf-m17n-ur.la \
	libnimf-m17n-uz.la \
	libnimf-m17n-vi.la \
	libnimf-m17n-yi.la

NIMF_M17N_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_M17N_DEPS_CFLAGS)

NIMF_M17N_ADD = \
	$(top_builddir)/libnimf/libnimf.la \
	$(top_builddir)/modules/engines/nimf-m17n/libnimf-m17n.la

gsettings_SCHEMAS = org.nimf.engines.nimf-m17n.gschema.xml

if M17N_1_8_0_AVAILABLE

module_LTLIBRARIES += \
	libnimf-m17n-hi.la \
	libnimf-m17n-hu.la \
	libnimf-m17n-kn.la \
	libnimf-m17n-ml.la \
	libnimf-m17n-mr.la \
	libnimf-m17n-ne.la

gsettings_SCHEMAS  += \
	org.nimf.engines.nimf-m17n-hi.gschema.xml \
	org.nimf.engines.nimf-m17n-hu.gschema.xml \
	org.nimf.engines.nimf-m17n-kn.gschema.xml \
	org.nimf.engines.nimf-m17n-ml.gschema.xml \
	org.nimf.engines.nimf-m17n-mr.gschema.xml \
	org.nimf.engines.nimf-m17n-ne.gschema.xml

libnimf_m17n_hi_la_SOURCES      = nimf-m17n-hi.c nimf-m17n.h
libnimf_m17n_hi_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_hi_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_hi_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_hi_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_hu_la_SOURCES      = nimf-m17n-hu.c nimf-m17n.h
libnimf_m17n_hu_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_hu_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_hu_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_hu_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_kn_la_SOURCES      = nimf-m17n-kn.c nimf-m17n.h
libnimf_m17n_kn_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_kn_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_kn_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_kn_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ml_la_SOURCES      = nimf-m17n-ml.c nimf-m17n.h
libnimf_m17n_ml_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ml_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ml_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ml_la_DEPENDENCIES      = libnimf-m17n.la

libnimf_m17n_mr_la_SOURCES      = nimf-m17n-mr.c nimf-m17n.h
libnimf_m17n_mr_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_mr_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_mr_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_mr_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ne_la_SOURCES      = nimf-m17n-ne.c nimf-m17n.h
libnimf_m17n_ne_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ne_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ne_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ne_la_DEPENDENCIES = libnimf-m17n.la

endif

@GSETTINGS_RULES@

libnimf_m17n_la_SOURCES = nimf-m17n.c nimf-m17n.h
libnimf_m17n_la_CFLAGS  = $(NIMF_M17N_CFLAGS)
libnimf_m17n_la_LDFLAGS = -avoid-version $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

libnimf_m17n_am_la_SOURCES      = nimf-m17n-am.c nimf-m17n.h
libnimf_m17n_am_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_am_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_am_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_am_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ar_la_SOURCES      = nimf-m17n-ar.c nimf-m17n.h
libnimf_m17n_ar_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ar_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ar_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ar_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_as_la_SOURCES      = nimf-m17n-as.c nimf-m17n.h
libnimf_m17n_as_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_as_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_as_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_as_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ath_la_SOURCES      = nimf-m17n-ath.c nimf-m17n.h
libnimf_m17n_ath_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ath_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ath_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ath_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_be_la_SOURCES      = nimf-m17n-be.c nimf-m17n.h
libnimf_m17n_be_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_be_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_be_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_be_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_bla_la_SOURCES      = nimf-m17n-bla.c nimf-m17n.h
libnimf_m17n_bla_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_bla_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_bla_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_bla_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_bn_la_SOURCES      = nimf-m17n-bn.c nimf-m17n.h
libnimf_m17n_bn_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_bn_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_bn_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_bn_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_bo_la_SOURCES      = nimf-m17n-bo.c nimf-m17n.h
libnimf_m17n_bo_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_bo_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_bo_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_bo_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_cmc_la_SOURCES      = nimf-m17n-cmc.c nimf-m17n.h
libnimf_m17n_cmc_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_cmc_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_cmc_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_cmc_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_cr_la_SOURCES      = nimf-m17n-cr.c nimf-m17n.h
libnimf_m17n_cr_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_cr_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_cr_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_cr_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_cs_la_SOURCES      = nimf-m17n-cs.c nimf-m17n.h
libnimf_m17n_cs_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_cs_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_cs_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_cs_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_da_la_SOURCES      = nimf-m17n-da.c nimf-m17n.h
libnimf_m17n_da_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_da_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_da_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_da_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_dv_la_SOURCES      = nimf-m17n-dv.c nimf-m17n.h
libnimf_m17n_dv_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_dv_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_dv_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_dv_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_el_la_SOURCES      = nimf-m17n-el.c nimf-m17n.h
libnimf_m17n_el_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_el_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_el_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_el_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_eo_la_SOURCES      = nimf-m17n-eo.c nimf-m17n.h
libnimf_m17n_eo_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_eo_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_eo_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_eo_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_fa_la_SOURCES      = nimf-m17n-fa.c nimf-m17n.h
libnimf_m17n_fa_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_fa_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_fa_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_fa_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_fr_la_SOURCES      = nimf-m17n-fr.c nimf-m17n.h
libnimf_m17n_fr_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_fr_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_fr_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_fr_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_grc_la_SOURCES      = nimf-m17n-grc.c nimf-m17n.h
libnimf_m17n_grc_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_grc_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_grc_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_grc_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_gu_la_SOURCES      = nimf-m17n-gu.c nimf-m17n.h
libnimf_m17n_gu_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_gu_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_gu_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_gu_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_he_la_SOURCES      = nimf-m17n-he.c nimf-m17n.h
libnimf_m17n_he_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_he_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_he_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_he_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_hr_la_SOURCES      = nimf-m17n-hr.c nimf-m17n.h
libnimf_m17n_hr_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_hr_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_hr_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_hr_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_hy_la_SOURCES      = nimf-m17n-hy.c nimf-m17n.h
libnimf_m17n_hy_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_hy_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_hy_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_hy_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ii_la_SOURCES      = nimf-m17n-ii.c nimf-m17n.h
libnimf_m17n_ii_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ii_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ii_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ii_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_iu_la_SOURCES      = nimf-m17n-iu.c nimf-m17n.h
libnimf_m17n_iu_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_iu_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_iu_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_iu_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ka_la_SOURCES      = nimf-m17n-ka.c nimf-m17n.h
libnimf_m17n_ka_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ka_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ka_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ka_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_kk_la_SOURCES      = nimf-m17n-kk.c nimf-m17n.h
libnimf_m17n_kk_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_kk_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_kk_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_kk_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_km_la_SOURCES      = nimf-m17n-km.c nimf-m17n.h
libnimf_m17n_km_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_km_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_km_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_km_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ks_la_SOURCES      = nimf-m17n-ks.c nimf-m17n.h
libnimf_m17n_ks_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ks_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ks_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ks_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_lo_la_SOURCES      = nimf-m17n-lo.c nimf-m17n.h
libnimf_m17n_lo_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_lo_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_lo_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_lo_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_mai_la_SOURCES      = nimf-m17n-mai.c nimf-m17n.h
libnimf_m17n_mai_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_mai_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_mai_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_mai_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_my_la_SOURCES      = nimf-m17n-my.c nimf-m17n.h
libnimf_m17n_my_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_my_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_my_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_my_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_nsk_la_SOURCES      = nimf-m17n-nsk.c nimf-m17n.h
libnimf_m17n_nsk_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_nsk_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_nsk_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_nsk_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_oj_la_SOURCES      = nimf-m17n-oj.c nimf-m17n.h
libnimf_m17n_oj_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_oj_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_oj_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_oj_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_or_la_SOURCES      = nimf-m17n-or.c nimf-m17n.h
libnimf_m17n_or_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_or_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_or_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_or_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_pa_la_SOURCES      = nimf-m17n-pa.c nimf-m17n.h
libnimf_m17n_pa_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_pa_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_pa_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_pa_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ps_la_SOURCES      = nimf-m17n-ps.c nimf-m17n.h
libnimf_m17n_ps_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ps_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ps_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ps_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ru_la_SOURCES      = nimf-m17n-ru.c nimf-m17n.h
libnimf_m17n_ru_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ru_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ru_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ru_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_sa_la_SOURCES      = nimf-m17n-sa.c nimf-m17n.h
libnimf_m17n_sa_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_sa_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_sa_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_sa_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_sd_la_SOURCES      = nimf-m17n-sd.c nimf-m17n.h
libnimf_m17n_sd_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_sd_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_sd_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_sd_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_si_la_SOURCES      = nimf-m17n-si.c nimf-m17n.h
libnimf_m17n_si_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_si_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_si_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_si_la_DEPENDENCIES      = libnimf-m17n.la

libnimf_m17n_sk_la_SOURCES      = nimf-m17n-sk.c nimf-m17n.h
libnimf_m17n_sk_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_sk_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_sk_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_sk_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_sr_la_SOURCES      = nimf-m17n-sr.c nimf-m17n.h
libnimf_m17n_sr_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_sr_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_sr_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_sr_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_sv_la_SOURCES      = nimf-m17n-sv.c nimf-m17n.h
libnimf_m17n_sv_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_sv_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_sv_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_sv_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_t_la_SOURCES      = nimf-m17n-t.c nimf-m17n.h
libnimf_m17n_t_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_t_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_t_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_t_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ta_la_SOURCES      = nimf-m17n-ta.c nimf-m17n.h
libnimf_m17n_ta_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ta_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ta_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ta_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_tai_la_SOURCES      = nimf-m17n-tai.c nimf-m17n.h
libnimf_m17n_tai_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_tai_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_tai_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_tai_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_te_la_SOURCES      = nimf-m17n-te.c nimf-m17n.h
libnimf_m17n_te_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_te_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_te_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_te_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_th_la_SOURCES      = nimf-m17n-th.c nimf-m17n.h
libnimf_m17n_th_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_th_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_th_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_th_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ug_la_SOURCES      = nimf-m17n-ug.c nimf-m17n.h
libnimf_m17n_ug_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ug_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ug_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ug_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_uk_la_SOURCES      = nimf-m17n-uk.c nimf-m17n.h
libnimf_m17n_uk_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_uk_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_uk_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_uk_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_ur_la_SOURCES      = nimf-m17n-ur.c nimf-m17n.h
libnimf_m17n_ur_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_ur_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_ur_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_ur_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_uz_la_SOURCES      = nimf-m17n-uz.c nimf-m17n.h
libnimf_m17n_uz_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_uz_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_uz_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_uz_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_vi_la_SOURCES      = nimf-m17n-vi.c nimf-m17n.h
libnimf_m17n_vi_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_vi_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_vi_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_vi_la_DEPENDENCIES = libnimf-m17n.la

libnimf_m17n_yi_la_SOURCES      = nimf-m17n-yi.c nimf-m17n.h
libnimf_m17n_yi_la_CFLAGS       = $(NIMF_M17N_CFLAGS)
libnimf_m17n_yi_la_LDFLAGS      = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_yi_la_LIBADD       = $(NIMF_M17N_ADD)
libnimf_m17n_yi_la_DEPENDENCIES = libnimf-m17n.la

check_PROGRAMS = test-nimf-m17n
TESTS = $(check_PROGRAMS)
test_nimf_m17n_SOURCES = test-nimf-m17n.c
test_nimf_m17n_CFLAGS  = $(NIMF_M17N_CFLAGS)
test_nimf_m17n_LDFLAGS = $(NIMF_M17N_DEPS_LIBS)
test_nimf_m17n_LDADD   = $(NIMF_M17N_ADD)

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-am.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-am.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ar.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ar.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-as.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-as.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ath.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ath.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-be.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-be.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-bla.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-bla.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-bn.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-bn.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-bo.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-bo.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-cmc.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-cmc.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-cr.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-cr.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-cs.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-cs.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-da.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-da.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-dv.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-dv.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-el.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-el.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-eo.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-eo.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-fa.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-fa.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-fr.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-fr.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-grc.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-grc.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-gu.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-gu.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-he.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-he.la
	-chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-hi.so -f
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hi.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-hr.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hr.la
	-chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-hu.so -f
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hu.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-hy.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hy.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ii.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ii.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-iu.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-iu.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ka.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ka.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-kk.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-kk.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-km.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-km.la
	-chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-kn.so -f
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-kn.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ks.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ks.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-lo.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-lo.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-mai.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-mai.la
	-chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ml.so -f
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ml.la
	-chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-mr.so -f
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-mr.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-my.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-my.la
	-chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ne.so -f
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ne.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-nsk.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-nsk.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-oj.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-oj.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-or.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-or.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-pa.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-pa.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ps.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ps.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ru.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ru.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-sa.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sa.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-sd.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sd.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-si.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-si.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-sk.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sk.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-sr.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sr.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-sv.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sv.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-t.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-t.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ta.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ta.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-tai.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-tai.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-te.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-te.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-th.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-th.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ug.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ug.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-uk.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-uk.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-ur.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ur.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-uz.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-uz.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-vi.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-vi.la
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n-yi.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-yi.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-am.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ar.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-as.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ath.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-be.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-bla.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-bn.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-bo.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-cmc.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-cr.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-cs.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-da.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-dv.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-el.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-eo.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-fa.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-fr.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-grc.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-gu.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-he.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hi.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hr.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hu.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-hy.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ii.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-iu.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ka.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-kk.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-km.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-kn.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ks.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-lo.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-mai.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ml.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-mr.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-my.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ne.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-nsk.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-oj.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-or.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-pa.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ps.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ru.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sa.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sd.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-si.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sk.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sr.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-sv.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-t.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ta.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-tai.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-te.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-th.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ug.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-uk.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-ur.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-uz.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-vi.so
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n-yi.so
	-rmdir -p $(DESTDIR)$(moduledir)
