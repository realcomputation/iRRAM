/*

iRRAM_REAL.h -- header file for the REAL class of the iRRAM library
 
Copyright (C) 2001-2013 Norbert Mueller
 
This file is part of the iRRAM Library.
 
The iRRAM Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.
 
The iRRAM Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.
 
You should have received a copy of the GNU Library General Public License
along with the iRRAM Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. 
*/

#ifndef iRRAM_REAL_H
#define iRRAM_REAL_H

#include <cmath>
#include <vector>

#include <iRRAM/helper-templates.hh>
#include <iRRAM/LAZYBOOLEAN.h>
#include <iRRAM/INTEGER.h>
#include <iRRAM/STREAMS.h> /* float_form, iRRAM_DEBUG* */

#ifdef __SSE2__at_the_moment_not_used_due_to_alignment_problems_in_gcc_4_3
#include <emmintrin.h>
#define _use_SSE2_
#endif

namespace iRRAM {

/*! \ingroup types */
class REAL final : conditional_comparison_overloads<REAL,LAZY_BOOLEAN>
{
	struct double_pair {
	#ifdef _use_SSE2__
		union {
			struct {
				double lower_pos, upper_neg;
			};
			__m128d sse_data;
		};
		double_pair(const __m128d& sse) noexcept : sse_data(sse) {}
	#else
		double lower_pos, upper_neg;
	#endif
		double_pair(const double l,const double u) noexcept
		: lower_pos(l), upper_neg(u) {}
		double_pair() noexcept {}
	};
public:

	// Constructors: -------------------------------

	REAL();
	REAL(      int           i);
	explicit REAL(const std::string  &s);
	explicit REAL(const char         *s);
	REAL(const DYADIC       &y);
	REAL(const REAL         &y);          /* copy constructor */
	REAL(      REAL        &&y) noexcept; /* move constructor */
	REAL(      double        d);
	REAL(const INTEGER      &y);
	REAL(const RATIONAL     &y);

	// Copy/Move Assignment: -----------------------

	REAL & operator=(const REAL& y);
	REAL & operator=(REAL &&y) noexcept;

	// Destructor: ---------------------------------

	~REAL();

	// Standard Arithmetic: ------------------------

	friend REAL operator+(const REAL &, const REAL &);

	REAL & operator+=(const REAL &);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator+(const A &a, const B &b);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator+(const B &b, const A &a);


	friend REAL operator-(const REAL &x, const REAL &y);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator-(const A &a, const B &b);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator-(const B &b, const A &a);

	/* double not optimized yet, maybe MPFR contains "x-d" and "d-x" */
	REAL operator-() const;
	REAL & operator-=(const REAL &y);
	REAL & operator-=(      int   n);

	friend REAL operator*(const REAL &x, const REAL &y);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator*(const A &a, const B &b);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator*(const B &b, const A &a);

	REAL & operator*=(const REAL &y) { return *this = *this * y; }
	REAL & operator*=(      int   n);

	friend REAL operator/(const REAL &x, const REAL &y);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator/(const A &a, const B &b);

	template <typename A,typename B>
	friend enable_if_compat<REAL,A,B> operator/(const B &b, const A &a);

	REAL & operator/=(const REAL &y) { return *this = *this / y; }
//	REAL & operator/=(      int   n);

	friend REAL   operator << (const REAL   &x,       int     n);
	friend REAL   operator >> (const REAL   &x,       int     n);

	friend REAL          sqrt        (const REAL &x);
	friend REAL          square      (const REAL &x);
	friend REAL          scale       (const REAL &x, const int k);

	// Comparisons: --------------------------------

	friend LAZY_BOOLEAN  operator <  (const REAL &x, const REAL &y);
	friend LAZY_BOOLEAN  operator <= (const REAL &x, const REAL &y);
	friend LAZY_BOOLEAN  operator >  (const REAL &x, const REAL &y);
	friend LAZY_BOOLEAN  operator >= (const REAL &x, const REAL &y);
	friend LAZY_BOOLEAN  operator == (const REAL &x, const REAL &y);
	friend LAZY_BOOLEAN  operator != (const REAL &x, const REAL &y);

	friend LAZY_BOOLEAN  positive    (const REAL& x, const int k);

	friend LAZY_BOOLEAN  bound       (const REAL& x, const int k);

	/* Conversions: */

	friend DYADIC approx(const REAL& x, const int p);

	DYADIC  as_DYADIC (const int p) const;
	DYADIC  as_DYADIC () const;

	double  as_double (const int p = 53) const;

	INTEGER as_INTEGER() const;

	// Output: -------------------------------------

	friend std::string swrite    (const REAL& x, const int p, const float_form form);

	friend int         upperbound(const REAL& x);
	friend int         size      (const REAL& x); 

	friend REAL        abs       (const REAL& x);

	void          rcheck      (int n=50) const;


// limit operators: ------------------------

friend REAL limit_lip (REAL f(int, const REAL&),
                       int lip,
                       bool on_domain(const REAL&),
                       const REAL& x);

friend REAL limit_lip (REAL f(int, const REAL&),
                       int lip(const REAL&),
                       const REAL& x);

friend REAL limit_lip (REAL f(int, const REAL&, const REAL&),
                       int lip,
                       bool on_domain(const REAL&,const REAL&),
                       const REAL& x,
                       const REAL& y);

// reduced error propagation: ------------------------

friend REAL lipschitz (REAL f(const REAL&),
                       int lip,
                       bool on_domain(const REAL&),
                       const REAL& x);

friend REAL lipschitz (REAL f(const REAL&),
                       REAL lip_f(const REAL&),
                       bool on_domain(const REAL&),
                       const REAL& x);

friend REAL lipschitz (REAL f(const REAL&,const REAL&),
                       int lip,
                       bool on_domain(const REAL&,const REAL&),
                       const REAL& x,
                       const REAL& y);

friend REAL lipschitz (REAL f(int, const REAL&),
                       int lip,
                       bool on_domain(int k,const REAL&),
                       int k,
                       const REAL& x);

friend REAL lipschitz (REAL f(int, const REAL&,const REAL&),
                       int lip,
                       bool on_domain(int k,const REAL&,const REAL&),
                       int k,
                       const REAL& x,
                       const REAL& y);

friend REAL lipschitz (REAL f(const REAL&),
                       REAL lip_f(const REAL&),
                       const REAL& x);

friend class TM;

friend void swap(REAL &, REAL &) noexcept;
friend REAL strtoREAL2(const char *s, char **endptr);

// implementational issues: --------------------
public:
	/*!
	 * \brief Closed `double` interval containg x.
	 *
	 * Let this REAL object denote \f$x\in\mathbb R\f$, then in this
	 * documentation `dp.lower_pos` is denoted as \f$x_L\f$, `dp.upper_neg`
	 * is denoted as \f$x_{-U}\f$ and likewise `-dp.upper_neg` is \f$x_U\f$.
	 * This way of approximating reals is only used in the \ref Iteration
	 * with `actual_step == 1`.
	 *
	 * \invariant `value == NULL` \f$\Longrightarrow x_L\leq x\leq x_U\f$.
	 */
	double_pair   dp;

	/*!
	 * \brief Center of the current interval containing the real value x.
	 *
	 * Let this REAL object denote \f$x\in\mathbb R\f$, then in this
	 * documentation `value` is referred to as \f$x_c\f$. It may differ from
	 * one \ref Iteration to the next.
	 *
	 * \invariant `value != NULL`
	 * \f$\Longrightarrow|x-x_c|\leq x_\varepsilon\f$
	 */
	MP_type       value;

	/*!
	 * \brief Radius of the current interval containing the real value x.
	 *
	 * Let this REAL object denote \f$x\in\mathbb R\f$, then in this
	 * documentation `error` is referred to as \f$x_\varepsilon\f$. It may
	 * very well differ from one \ref Iteration to the next.
	 *
	 * \invariant `value != NULL`
	 * \f$\Longrightarrow|x-x_c|\leq x_\varepsilon\f$ */
	sizetype      error;

	/*!
	 * \brief Tight \ref sizetype typed approximation to `value`.
	 *
	 * Let this REAL object denote \f$x\in\mathbb R\f$, then in this
	 * documentation `vsize` is referred to as \f$\hat x\f$. It may differ
	 * from one \ref Iteration to the next. Additionally, for non-zero
	 * \f$\hat x\f$, we refer to \f$\check x=(m-1)\cdot2^e\f$, the value of
	 * which depends on the \ref sizetype representation of
	 * \f$\hat x=m\cdot2^e\f$.
	 *
	 * \invariant `value != NULL`
	 * \f$\Longrightarrow\texttt{vsize}=m\cdot2^e:(m-1)\cdot2^e=\check x
	 *                                 \leq|x_c|<\hat x=m\cdot2^e\f$
	 */
	sizetype      vsize;

public:
	void         adderror           (sizetype  error);
	void         seterror           (sizetype  error);
	void         geterror           (sizetype &error) const;
	sizetype     geterror           ()                const { sizetype t; geterror(t); return t; }
	void         getsize            (sizetype &error) const;
	sizetype     getsize            ()                const { sizetype t; getsize(t); return t; }
	void         to_formal_ball     (DYADIC &, sizetype &error) const;

//	friend REAL intervall_join (const REAL& x,const REAL& y);

// internal use:
private:
	REAL(MP_type y, sizetype errorinfo) noexcept;
	REAL(const double_pair &ydp) noexcept;
#ifdef _use_SSE2__
	REAL(const __m128d     &y_sse) noexcept;
#endif

	void         mp_copy            (const REAL   &);
	void         mp_copy_init       (const REAL   &);
	void         mp_make_mp         ();
	void         mp_from_mp         (const REAL   &y);
	void         mp_from_int        (const int     i);
	void         mp_from_double     (const double  d);
	REAL &       mp_conv            ()                const;
	REAL         mp_addition        (const REAL   &y) const;
	REAL         mp_addition        (const int     i) const;
	REAL &       mp_eqaddition      (const REAL   &y);
//	REAL         mp_addition        (const double  i) const; //fehlt noch
	REAL         mp_subtraction     (const REAL   &y) const;
	REAL         mp_subtraction     (const int     i) const;
	REAL         mp_invsubtraction  (const int     i) const;
	REAL         mp_multiplication  (const REAL   &y) const;
	REAL         mp_multiplication  (const int     y) const;
	REAL &       mp_eqmultiplication(const REAL   &y);
	REAL &       mp_eqmultiplication(const int     i);
//	REAL         mp_multiplication  (const double  y) const; //fehlt noch
	REAL         mp_division        (const REAL   &y) const;
	REAL         mp_division        (const int     y) const;
	REAL         mp_division        (const double  y) const;
	REAL         mp_square          ()                const;
	REAL         mp_absval          ()                const;
	REAL         mp_intervall_join  (const REAL   &y) const;
	LAZY_BOOLEAN mp_less            (const REAL   &y) const;
};

/*! \relates REAL */
inline sizetype geterror(const REAL &r) { return r.geterror(); }
/*! \relates REAL */
inline void     seterror(REAL &r, const sizetype &err) { r.seterror(err); }

inline void     adderror(REAL &r, const sizetype &err) { r.adderror(err); }

inline void     to_formal_ball(const REAL &r, DYADIC &center, sizetype &err)
{
	r.to_formal_ball(center, err);
}

// for the sake of proving computational adequacy:
// if q=module(f,x,p), then |z-x|<2^q => |f(z)-f(x)| < 2^p
//! \related REAL
int module(REAL (*f)(const REAL&),const REAL& x, int p);

REAL strtoREAL(const char* s, char** endptr);
REAL atoREAL(const char* s);
REAL strtoREAL2(const char *s, char **endptr);

inline REAL round2(const REAL& x) { return REAL(x.as_INTEGER()); }
inline int  round (const REAL& x) { return int (x.as_INTEGER()); }

/****************************************************************************/
// arithmetic functions
/****************************************************************************/
/*! \addtogroup maths
 * @{ */
REAL power   (const REAL& x, const REAL& y);
REAL power   (const REAL& x, int n);
REAL modulo  (const REAL& x, const REAL& y);
REAL maximum (const REAL& x, const REAL& y);
REAL minimum (const REAL& x, const REAL& y);

/****************************************************************************/
// roots
/****************************************************************************/
REAL sqrt    (const REAL& x);
REAL root    (const REAL& x,int n);

/*! \addtogroup trigonometry
 * @{ */
/****************************************************************************/
// trigonometric functions
/****************************************************************************/
REAL sin     (const REAL& x);
REAL cos     (const REAL& x);
REAL tan     (const REAL& x);
REAL cotan   (const REAL& x);
REAL sec     (const REAL& x);
REAL cosec   (const REAL& x);

/****************************************************************************/
// inverse trigonometric functions
/****************************************************************************/
REAL atan    (const REAL& x);
REAL asin    (const REAL& x);
REAL acos    (const REAL& x);
REAL acotan  (const REAL& x);
REAL asec    (const REAL& x);
REAL acosec  (const REAL& x);

/****************************************************************************/
//hyperbolic functions
/****************************************************************************/
REAL sinh    (const REAL& x);
REAL cosh    (const REAL& x);
REAL tanh    (const REAL& x);
REAL coth    (const REAL& x);
REAL sech    (const REAL& x);
REAL cosech  (const REAL& x);

/****************************************************************************/
// inverse hyperbolic functions 
/****************************************************************************/
REAL asinh   (const REAL& x);
REAL acosh   (const REAL& x);
REAL atanh   (const REAL& x);
REAL acoth   (const REAL& x);
REAL asech   (const REAL& x);
REAL acosech (const REAL& x);
//! @}

/****************************************************************************/
// exponentiation + logarithm
/****************************************************************************/
REAL exp     (const REAL& x);
REAL log     (const REAL& x);

/****************************************************************************/
// special constants values
/****************************************************************************/
REAL pi      ();   // = 3.141592653...
REAL euler   ();   // = 2.718281828...
REAL ln2     ();   // = 0.693147180...

/****************************************************************************/
//  a few vector functions
/****************************************************************************/
REAL abs    (const std::vector<REAL>& x);
//! @}

void rwrite (const REAL& x, const int w);
void rwritee(const REAL& x, const int w);
void rshow  (const REAL& x, const int w);

std::string swrite(const REAL & x, const int p,
                   const float_form form = float_form::absolute);

// inlined versions of most important functions:

//"private" internal  constructor
inline REAL::REAL(MP_type y, sizetype errorinfo) noexcept
: value(y), error(errorinfo)
{
    MP_getsize(value,vsize);
}

//"private" internal  constructor
inline REAL::REAL(const double_pair& ydp) noexcept
: dp(ydp), value(nullptr) {}

#ifdef _use_SSE2__
inline REAL::REAL(const __m128d& y_sse) noexcept
: dp(y_sse), value(nullptr) {}
#endif

inline REAL::~REAL() 
{
	if (iRRAM_unlikely(value)) {
		MP_clear(value);
		value = nullptr;
	}
}

inline REAL::REAL() : REAL(0) {}

inline REAL::REAL(int i) : dp((double)i,-(double)i), value(nullptr)
{
	if (iRRAM_unlikely(state->highlevel))
		mp_from_int(i);
}

inline REAL::REAL(double d) : dp(d,-d), value(nullptr)
{
	if (!std::isfinite(d))
		throw iRRAM_Numerical_Exception(iRRAM_conversion_from_infinite);
	if (iRRAM_unlikely(state->highlevel))
		mp_from_double(d);
}

inline REAL::REAL(const REAL& y) : dp(y.dp), value(nullptr)
{
	if (iRRAM_unlikely(y.value))
		mp_copy_init(y);
}

inline REAL::REAL(REAL &&y) noexcept
: dp(y.dp), value(y.value), error(y.error), vsize(y.vsize)
{
	y.value = nullptr;
}

inline REAL & REAL::mp_conv() const
{
	if (!value)
		const_cast<REAL&>(*this).mp_make_mp();
	return const_cast<REAL&>(*this);
}

inline REAL & REAL::operator=(const REAL & y)
{
	if (iRRAM_unlikely(value || y.value)) {
		if (value && y.value) {
			this->mp_copy(y);
			return (*this);
		}
		if (y.value) {
			if (state->highlevel)
				this->mp_copy_init(y);
			else
				this->mp_from_mp(y);
			return *this;
		}
		dp = y.dp;
		mp_make_mp();
		return *this;
	}
	dp = y.dp;
	return *this;
}

inline void swap(REAL &a, REAL &b) noexcept
{
	using std::swap;
	swap(a.value, b.value);
	swap(a.dp   , b.dp);
	swap(a.error, b.error);
	swap(a.vsize, b.vsize);
}

/* TODO: what are iRRAM's semantics of REAL assignment?
 * Take the highest MPFR precision if ACTUAL_STACK.prec_step > iRRAM_DEFAULT_PREC_START?
inline REAL & REAL::operator=(REAL &&y) noexcept
{
	if (this == &y)
		return *this;
	if (iRRAM_unlikely(value||y.value)) {
		if (value && y.value) {
			swap(value, y.value);
			vsize = y.vsize;
			error = y.error;
			return *this;
		}
		if (y.value) {
			if (state->highlevel)
				
			else
				mp_from_mp(y);
		}
	}
	dp = y.dp;
	return *this;
}
*/
inline REAL & REAL::operator=(REAL &&y) noexcept
{
	using std::swap;
	swap(value, y.value);
	if (iRRAM_unlikely(value)) {
		vsize = y.vsize;
		error = y.error;
	} else
		dp = y.dp;
	return *this;
}

inline REAL operator<<(const REAL& x, int n) { return scale(x, n); }
inline REAL operator>>(const REAL& x, int n) { return scale(x,-n); }

inline REAL operator+(const REAL& x, const REAL& y)
{
	if (iRRAM_unlikely(x.value||y.value))
		return x.mp_conv().mp_addition(y.mp_conv());
#ifdef _use_SSE2__
	return REAL(_mm_add_pd(x.dp.sse_data,y.dp.sse_data));
#else
	return REAL(REAL::double_pair(x.dp.lower_pos+y.dp.lower_pos,
	                              x.dp.upper_neg+y.dp.upper_neg));
#endif
}

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator+(const A &a, const B &b) { return a+REAL(b); }

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator+(const B &b, const A &a) { return a+b; }

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator-(const A &a, const B &b) { return a-REAL(b); }

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator-(const B &b, const A &a) { return REAL(b)-a; }

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator*(const A &a, const B &b) { return a*REAL(b); }

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator*(const B &b, const A &a) { return a*b; }

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator/(const A &a, const B &b) { return a/REAL(b); }

template <typename A,typename B>
enable_if_compat<REAL,A,B> operator/(const B &b, const A &a) { return REAL(b)/a; }

template <>
inline REAL operator+(const REAL &x, const int &i)
{
	if (iRRAM_unlikely(x.value))
		return x.mp_addition(i);
	return REAL(REAL::double_pair(x.dp.lower_pos+i,
	                              x.dp.upper_neg-i));
}

inline REAL & REAL::operator+=(const REAL &y)
{
	if (iRRAM_unlikely(value||y.value)) {
		mp_conv().mp_eqaddition(y.mp_conv());
		return *this;
	}
#ifdef _use_SSE2__
	dp.sse_data = _mm_add_pd(x.dp.sse_data,y.dp.sse_data);
#else
	dp.lower_pos+=y.dp.lower_pos;
	dp.upper_neg+=y.dp.upper_neg;
#endif
	return *this;
}

inline REAL operator-(const REAL& x, const REAL& y)
{
	if (iRRAM_unlikely(x.value||y.value))
		return x.mp_conv().mp_subtraction(y.mp_conv());
	return REAL(REAL::double_pair(x.dp.lower_pos+y.dp.upper_neg,
	                              x.dp.upper_neg+y.dp.lower_pos));
}

template <>
inline REAL operator-(const REAL& x, const int &n)
{
	if (iRRAM_unlikely(x.value))
		return x.mp_subtraction(n);
	return REAL(REAL::double_pair(x.dp.lower_pos-n,
	                              x.dp.upper_neg+n));
}

template <>
inline REAL operator-(const int &n, const REAL& x)
{
	if (iRRAM_unlikely(x.value))
		return x.mp_invsubtraction(n);
	return REAL(REAL::double_pair(x.dp.upper_neg+n,
	                              x.dp.lower_pos-n));
}

inline REAL REAL::operator-() const
{
	if (iRRAM_unlikely(value))
		return mp_invsubtraction(int(0));
	return REAL(REAL::double_pair(-dp.lower_pos,
	                              -dp.upper_neg));
}

inline REAL & REAL::operator-=(const REAL &y) { return *this = *this - y; } /* TODO: specialize */
inline REAL & REAL::operator-=(      int   n) { return *this = *this - n; } /* TODO: specialize */

// inline double my_fmin(const double& x,const double& y)
// { return  x<y?x:y ; }

inline REAL operator*(const REAL & x, const REAL & y)
{
	if (iRRAM_unlikely(x.value || y.value))
		return x.mp_conv().mp_multiplication(y.mp_conv());
	REAL::double_pair z;
	if (x.dp.lower_pos >= 0 && y.dp.lower_pos >= 0) {
		z.lower_pos =   x.dp.lower_pos  * y.dp.lower_pos;
		z.upper_neg = (-x.dp.upper_neg) * y.dp.upper_neg;
	} else if (x.dp.upper_neg >= 0 && y.dp.upper_neg >= 0) {
		z.lower_pos =   x.dp.upper_neg  * y.dp.upper_neg;
		z.upper_neg = (-x.dp.lower_pos) * y.dp.lower_pos;
	} else if (x.dp.upper_neg >= 0 && y.dp.lower_pos >= 0) {
		z.lower_pos = x.dp.lower_pos * (-y.dp.upper_neg);
		z.upper_neg = x.dp.upper_neg *   y.dp.lower_pos;
	} else if (x.dp.lower_pos >= 0 && y.dp.upper_neg >= 0) {
		z.lower_pos = (-x.dp.upper_neg) * y.dp.lower_pos;
		z.upper_neg =   x.dp.lower_pos  * y.dp.upper_neg;
	} else {
		z.lower_pos = fmin((-x.dp.upper_neg) *   y.dp.lower_pos,
		                     x.dp.lower_pos  * (-y.dp.upper_neg));
		z.upper_neg = fmin((-x.dp.upper_neg) *   y.dp.upper_neg,
		                     x.dp.lower_pos  * (-y.dp.lower_pos));
	}
	return REAL(z);
}

template <>
inline REAL operator*(const REAL& x, const int &n)
{
	if (iRRAM_unlikely(x.value))
		return x.mp_multiplication(n);
	REAL::double_pair z;
	if (n >= 0) {
		z.lower_pos = x.dp.lower_pos * n;
		z.upper_neg = x.dp.upper_neg * n;
	} else {
		z.lower_pos = (-x.dp.upper_neg) * n;
		z.upper_neg = (-x.dp.lower_pos) * n;
	}
	return REAL(z);
}

inline REAL & REAL::operator*=(int n)
{
	if (iRRAM_unlikely(value))
		return *this = mp_multiplication(n);
	if (n >= 0) {
		dp.lower_pos *= n;
		dp.upper_neg *= n;
	} else {
		double tmp    = -dp.upper_neg * n;
		dp.upper_neg  = -dp.lower_pos * n;
		dp.lower_pos  = tmp;
	}
	return *this;
}

inline REAL operator/(const REAL& x, const REAL& y)
{
	if (iRRAM_unlikely(x.value||y.value))
		return x.mp_conv().mp_division(y.mp_conv());
	REAL::double_pair z;
	if (y.dp.lower_pos > 0.0) {
		if (x.dp.lower_pos > 0.0) {
			z.lower_pos = x.dp.lower_pos/(-y.dp.upper_neg);
			z.upper_neg = x.dp.upper_neg/  y.dp.lower_pos;
		} else if (x.dp.upper_neg  > 0.0){
			z.lower_pos = x.dp.lower_pos/  y.dp.lower_pos;
			z.upper_neg = x.dp.upper_neg/(-y.dp.upper_neg);
		} else {
			z.lower_pos = x.dp.lower_pos/  y.dp.lower_pos;
			z.upper_neg = x.dp.upper_neg/  y.dp.lower_pos;
		}
	} else if (y.dp.upper_neg > 0.0) {
		if (x.dp.lower_pos > 0.0 ) {
			z.lower_pos =   x.dp.upper_neg /y.dp.upper_neg;
			z.upper_neg = (-x.dp.lower_pos)/y.dp.lower_pos;
		} else if (x.dp.upper_neg  > 0.0){
			z.lower_pos = (-x.dp.upper_neg)/y.dp.lower_pos;
			z.upper_neg =   x.dp.lower_pos /y.dp.upper_neg;
		} else {
			z.lower_pos = x.dp.upper_neg   /y.dp.upper_neg;
			z.upper_neg = x.dp.lower_pos   /y.dp.upper_neg;
		}
	} else {
		return x.mp_conv().mp_division(y.mp_conv()); // containing zero...
	}
	return REAL(z);
}

template <>
inline REAL operator/(const REAL& x, const int &n)
{
	if (iRRAM_unlikely(x.value))
		return x.mp_division(n);
	REAL::double_pair z;
	if (n > 0) {
#ifdef _use_SSE2__
		__m128d n_sse = _mm_set_pd1(double(n));
		n_sse = _mm_div_pd(x.dp.sse_data, n_sse);
		return REAL(n_sse);
#else
		z.lower_pos = x.dp.lower_pos / n,
		z.upper_neg = x.dp.upper_neg / n;
#endif
	} else if (n < 0) {
		z.lower_pos = (-x.dp.upper_neg) / n;
		z.upper_neg = (-x.dp.lower_pos) / n;
	} else {
		return x.mp_conv().mp_division(0); // containing zero...
	}
	return REAL(z);
}


inline REAL square(const REAL & x)
{
	if (iRRAM_unlikely(x.value)) {
		return x.mp_square();
	}
	REAL::double_pair z;
	if (x.dp.lower_pos >= 0) {
		z.lower_pos =   x.dp.lower_pos  *   x.dp.lower_pos;
		z.upper_neg = (-x.dp.upper_neg) *   x.dp.upper_neg;
	} else if (x.dp.upper_neg >= 0) {
		z.lower_pos =   x.dp.upper_neg  *   x.dp.upper_neg;
		z.upper_neg = (-x.dp.lower_pos) *   x.dp.lower_pos;
	} else if (x.dp.lower_pos < x.dp.upper_neg) {
		z.lower_pos = 0.0;
		z.upper_neg =   x.dp.lower_pos  * (-x.dp.lower_pos);
	} else {
		z.lower_pos = 0.0;
		z.upper_neg = (-x.dp.upper_neg) *   x.dp.upper_neg;
	}
	return REAL(z);
}

inline LAZY_BOOLEAN operator<(const REAL & x, const REAL & y)
{
	if (iRRAM_unlikely(x.value || y.value))
		return x.mp_conv().mp_less(y.mp_conv());
	if ((-x.dp.upper_neg) <   y.dp.lower_pos )
		return true;
	if (  x.dp.lower_pos  > (-y.dp.upper_neg))
		return false;
	return LAZY_BOOLEAN::BOTTOM;
}

inline LAZY_BOOLEAN operator<=(const REAL &x, const REAL &y) { return (x<y); }
inline LAZY_BOOLEAN operator> (const REAL &x, const REAL &y) { return (y<x); }
inline LAZY_BOOLEAN operator>=(const REAL &x, const REAL &y) { return (y<x); }
inline LAZY_BOOLEAN operator==(const REAL &x, const REAL &y) { return (y<x)&&(x<y); }
inline LAZY_BOOLEAN operator!=(const REAL &x, const REAL &y) { return (y<x)||(x<y); }

inline REAL abs(const REAL & x)
{
	if (iRRAM_unlikely(x.value))
		return x.mp_absval();
	if (x.dp.lower_pos > 0.0)
		return REAL(REAL::double_pair(x.dp.lower_pos, x.dp.upper_neg));
	if (x.dp.upper_neg > 0.0)
		return REAL(REAL::double_pair(x.dp.upper_neg, x.dp.lower_pos));
	if (x.dp.lower_pos > x.dp.upper_neg)
		return REAL(REAL::double_pair(0.0, x.dp.upper_neg));
	return REAL(REAL::double_pair(0.0, x.dp.lower_pos));
}

// inline REAL intervall_join (const REAL& x,const REAL& y){
//     if ( iRRAM_unlikely ( x.value||y.value ) )
// 	 { return x.mp_conv().mp_intervall_join(y.mp_conv()); }
//     double_pair z;
//     if (x.dp.lower_pos < y.dp.lower_pos )
//       z.dp.lower_pos=x.dp.lower_pos
//     else
//       z.dp.lower_pos=y.dp.lower_pos;
//     if (x.dp.upper_neg < y.dp.upper_neg )
//       z.dp.upper_neg=x.dp.upper_neg
//     else
//       z.dp.upper_neg=y.dp.upper_neg;
//     return REAL(z);
// }

} // namespace iRRAM

#endif
