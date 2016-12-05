
#ifndef iRRAM_COMMON_H
#define iRRAM_COMMON_H

#if defined(__GNUC__) || defined(__clang__) || defined(__builtin_expect)
/* Don't need to check version of compilers, they all support __builtin_expect()
 * since the time they know of C++11 */
# define iRRAM_expect(e,n)	__builtin_expect((e),(n))
#else
# define iRRAM_expect(e,n)	(e)
#endif
#define iRRAM_likely(x)		iRRAM_expect(!!(x), 1)
#define iRRAM_unlikely(x)	iRRAM_expect(!!(x), 0)

#include <iRRAM/version.h>

#ifndef iRRAM_BACKENDS
# error error: no usable backend, defined iRRAM_BACKENDS
#endif

#if iRRAM_BACKEND_MPFR
# include <iRRAM/MPFR_interface.h>
#else
# error "Currently no additional backend!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void iRRAM_initialize(int argc, char **argv);

/*! \brief like iRRAM_initialize(), but modifies its arguments as to remove
 *         parsed options pertaining iRRAM. */
void iRRAM_initialize2(int *argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
