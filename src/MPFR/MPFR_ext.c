
#include <stdlib.h>
#include <stddef.h>

#include <iRRAM/MPFR_interface.h>

iRRAM_TLS struct iRRAM_ext_mpfr_cache_t *iRRAM_ext_mpfr_cache;

void ext_mpfr_initialize()
{
	mpfr_set_default_prec(32);

	iRRAM_ext_mpfr_cache = calloc(1, sizeof(*iRRAM_ext_mpfr_cache));
}
