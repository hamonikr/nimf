check_PROGRAMS = test-nimf

TESTS = $(check_PROGRAMS)

test_nimf_SOURCES = test-nimf.c

test_nimf_CFLAGS = \
	-Wall -Werror \
	$(TEST_NIMF_DEPS_CFLAGS)

test_nimf_LDFLAGS = $(TEST_NIMF_DEPS_LIBS)

DISTCLEANFILES = Makefile.in
