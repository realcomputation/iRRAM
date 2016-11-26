## Process this file with automake to produce Makefile.in
# echo -e "\n***  Compiling iRRAM with MPFR multiple precision package as backend ***\n"

srcdir = @srcdir@
VPATH = ../src

main_sources = \
	REALS.cc \
	REALmain.cc \
	REALLIB.cc \
	limits.cc \
	errno.cc \
	convert.cc \
	stack.cc \
	STREAMS.cc \
	DYADIC.cc \
	COMPLEX.cc \
	RATIONAL.cc \
	LAZY_BOOLEAN.cc \
	sqrt.cc \
	exp_log.cc \
	sin_cos.cc \
	pi_ln2.cc \
	REALMATRIX.cc \
	SPARSEREALMATRIX.cc \
	INTERVAL.cc \
	GMP_int_ext.c \
	GMP_rat_ext.c

mpfr_sources = \
	MPFR_ext.c \
	mpfr_extension.cc

mpfr_headers = \
	MPFR_ext.h

DISTCLEANFILES = @iRRAM_src@

lib_LTLIBRARIES      = libiRRAM.la
libiRRAM_la_LDFLAGS  = -version-info @iRRAM_shared_version@ -no-undefined $(AM_LDFLAGS)
libiRRAM_la_SOURCES  = $(main_sources)

if WITH_BACKEND_MPFR
libiRRAM_la_SOURCES += $(mpfr_sources)
DISTCLEANFILES      += $(mpfr_sources) $(mpfr_headers)
endif
