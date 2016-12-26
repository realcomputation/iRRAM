
#include <cstdio>
#include <cstdlib>

#include <iRRAM/REAL.h>
#include <iRRAM/limit_templates.h>

#if iRRAM_BACKEND_MPFR
# include "MPFR/MPFR_ext.h"
#else
# error "Currently no additional backend!"
#endif

namespace iRRAM {

static REAL root_approx(int prec, const REAL & x, int n)
{
	int u = upperbound(x) / n;
	REAL error;
	if (u < prec)
		return 0;
	REAL r = scale(REAL(1), u);
	REAL r_2 = x / power(r, n - 1);
	do {
		r += (r_2 - r) / n;
		r_2 = x / power(r, n - 1);
		error = r - r_2;
	} while (!bound(error, prec));
	return r;
}

REAL root(const REAL & x, int n) { return limit(root_approx, x, n); }

#ifdef OLDSQRT
#ifdef MP_mv_sqrt

static REAL MP_sqrt_approx(int prec, const REAL & x)
{
	DYADIC xd = approx(x, prec - 1);
	MP_mv_sqrt(xd.value, xd.value, prec - 1);
	return REAL(xd);
};
#define SQRT_APPROX MP_sqrt_approx

#else

static REAL gen_sqrt_approx(int k, const REAL & x)
{
	REAL a = 1, b = x / a;
	do {
		a = (a + b) / 2;
		b = x / a;
	} while (!bound(a - b, k));
	return a;
}
#define SQRT_APPROX gen_sqrt_approx

#endif

static REAL sqrt_approx(int prec, const REAL & x)
{
	if (bound(x, 2 * prec))
		return 0;
	int s2 = size(x) / 2;
	REAL y = scale(x, -s2 * 2);
	y = lipschitz(SQRT_APPROX, 2, prec - s2, y);
	y = scale(y, s2);
	return y;
}

REAL sqrt(const REAL & x) { return limit(sqrt_approx, x); }

#else /* !OLDSQRT */


/*!
 * \brief Computes the square root of the argument \a x.
 *
 * For domain \f$[A;B]\f$ with \f$A>0\f$, the square root function
 * \f$f=\sqrt{}\f$ has a Lipschitz bound:
 * \f[ \forall x,y\in[A;B]:
 *     |f(x)-f(y)| \leq K\cdot|x-y| = K\cdot|f(x)+f(y)|\cdot|f(x)-f(y)|
 *     \Longleftrightarrow 1/|f(x)+f(y)| \leq K
 * \f]
 * Since \f$\max_{x,y \in [A;B]} 1/|f(x)+f(y)| = 1/|2f(A)|\leq K\f$ for the
 * function's argument \f$x\in[x_c\pm x_\varepsilon]\f$ the error
 * therefore is
 * \f$|f(x_c)-f(x)| \leq K|x-x_c| < Kx_\varepsilon
 *                  \leq x_\varepsilon/(2\sqrt{\check x-x_\varepsilon})\f$.
 *
 * This case, \f$x\geq A=\check x-x_\varepsilon>0\f$, is chosen when
 * \f$\check x\geq 4x_\varepsilon\Longrightarrow |x|\geq 3x_\varepsilon\f$,
 * therefore \f$|\sqrt{x_c}-\sqrt x|
 * \leq x_\varepsilon/(2\sqrt{\check x-x_\varepsilon})\f$.
 * The absolute precision used to compute
 * \f$\sqrt{x_c}\f$ is \f$\max\{P,e[z]\}\f$ where
 * \f$z\geq x_\varepsilon/(2\sqrt{\check x})\f$ and
 * \f$e[v]=e\f$ for any \ref sizetype representation \f$m\cdot2^e\f$ of \f$v\f$.
 * The relative precision is \f$\max\{e[\check x]+P,e[z]\}\f$.
 *
 * Otherwise, if \f$x\neq0\f$,
 * \f$4x_\varepsilon\geq\check x=\hat x-2^e>\hat x\cdot(1-2^{-32})\f$,
 * therefore \f$\sqrt{|x|}
 * <\sqrt{\hat x+x_\varepsilon}
 * <\sqrt{4x_\varepsilon/(1-2^{-32})+x_\varepsilon}
 * <2.25\sqrt{x_\varepsilon}\f$
 * and \f$0\approx\sqrt{|x|}\f$ is chosen as an approximation with error
 * \f$\geq3\sqrt{x_\varepsilon}\f$.
 *
 * \bug
 * Assignment of \f$z\f$ as \f$x_\varepsilon/(2\sqrt{\check x})
 * <x_\varepsilon/(2\sqrt{\check x-x_\varepsilon})\f$.
 *
 * \todo
 * For \f$0\in[x_c\pm x_\varepsilon]\f$ use
 * \f$\approx[\sqrt{x_\varepsilon}\pm\sqrt{x_\varepsilon}]\f$ as result to decrease
 * the error and disable representation of non-co-domain values smaller than
 * zero.
 *
 * \todo
 * To determine \f$0\notin[x_c\pm x_\varepsilon]\f$ check
 * \f$\check x\geq cx_\varepsilon\f$ with \f$c\in\{2,3\}\f$ for tighter error
 * bounds in both cases.
 *
 * \param x non-negative real number
 * \return sqrt(\a x)
 */
REAL sqrt(const REAL & x)
{
	if (!x.value)
		(const_cast<REAL &>(x)).mp_make_mp();
	MP_type zvalue;
	sizetype zerror, xsize, proderror;
	int local_prec;
	x.getsize(xsize);
	sizetype_dec(xsize);
	if (!x.vsize.mantissa || sizetype_less(xsize, x.error << 2)) {
		MP_init(zvalue);
		MP_int_to_mp(0, zvalue);
		sizetype_sqrt(zerror, x.error);
		zerror += zerror << 1;
		return REAL(zvalue, zerror);
	}
	sizetype_sqrt(proderror, xsize);
	sizetype_div(zerror, x.error, proderror);
	sizetype_half(zerror, zerror);
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(state.ACTUAL_STACK.actual_prec, zerror.exponent);
	else
		local_prec = max(xsize.exponent + state.ACTUAL_STACK.actual_prec,
		                 zerror.exponent);
	MP_init(zvalue);
	MP_mv_sqrt(x.value, zvalue, local_prec);
	zerror = sizetype_add_power2(zerror, local_prec);
	// printf("%d*2^%d\n",zerror.mantissa,zerror.exponent);
	return REAL(zvalue, zerror);
}
#endif

} // namespace iRRAM
