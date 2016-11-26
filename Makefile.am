## Process this file with automake to produce Makefile.in 
# SUBDIRS=include . examples
# doc still missing...

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc include src tests

.PHONY: examples

#LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

# The following functionality is discouraged by the automake authors, see
# <http://www.gnu.org/software/automake/manual/html_node/Scripts_002dbased-Testsuites.html>
#all-recursive: check

# copied shamelessly from GMP
install-data-hook:
	@echo ''
	@echo '+-------------------------------------------------------------+'
	@echo '| CAUTION:                                                    |'
	@echo '|                                                             |'
	@echo '| If you have not already run "make check", then we strongly  |'
	@echo '| recommend you do so.                                        |'
	@echo '|                                                             |'
	@echo '+-------------------------------------------------------------+'
	@echo ''

examples: install
	@echo -e "\n***  Compiling examples ***\n"
	(cd examples && $(MAKE))

# Still missing functionality:
#LIBTOOLFLAGS+=--silent

clean-local:
	(cd examples && $(MAKE) clean)

distclean-local:
	rm -f m4/libtool.m4  m4/lt~obsolete.m4  m4/ltoptions.m4  m4/ltsugar.m4  m4/ltversion.m4
#	rm -f test-driver missing install-sh ltmain.sh compile config.guess  config.sub depcomp
