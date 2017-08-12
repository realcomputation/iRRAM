
#include <stdlib.h>
#include <stddef.h>

#include <iRRAM/MPFR_interface.h>

iRRAM_TLS struct iRRAM_ext_mpfr_cache_t *iRRAM_ext_mpfr_cache;

void ext_mpfr_initialize()
{
	mpfr_set_default_prec(32);

	iRRAM_ext_mpfr_cache = calloc(1, sizeof(*iRRAM_ext_mpfr_cache));
}

void ext_mpfr_finalize(void)
{
	struct iRRAM_ext_mpfr_cache_t *cache = iRRAM_ext_mpfr_cache;
	for (size_t i=cache->free_var_count; i; i--) {
		mpfr_clear(cache->free_vars[i-1]);
		free(cache->free_vars[i-1]);
		cache->total_freed_var_count++;
	}
	free(cache);
	iRRAM_ext_mpfr_cache = NULL;
}
