/*
 * sizetype.hh -- define type representing errors and size bounds
 *
 * Copyright (C) 2001-2009 Norbert Mueller
 * Copyright     2014-2016 Franz Brausse
 *
 * This file is part of the iRRAM Library.
 *
 * The iRRAM Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * The iRRAM Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with the iRRAM Library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef SIZETYPE_HH
#define SIZETYPE_HH

#include <limits>
#include <cmath>

#include <iRRAM/core.h>

namespace iRRAM {

typedef typename sizetype::mantissa_t SIZETYPEMANTISSA;
typedef typename sizetype::exponent_t SIZETYPEEXPONENT;

const int MANTISSA_BITS = (int) (8*sizeof(SIZETYPEMANTISSA)) ;
const int DIFF_BITS     = 3;
const int BIT_RANGE     = 8;
const int BIT_RANGE2    = 8;
const int GUARD_BITS    =  MANTISSA_BITS - DIFF_BITS;

const SIZETYPEMANTISSA max_mantissa= 1 <<  GUARD_BITS   ;
const SIZETYPEMANTISSA min_mantissa= 1 << (GUARD_BITS-BIT_RANGE);

/* if the exponent of value is smaller than MP_min, it should be increased(!) to (1,min_exponent) */
const SIZETYPEEXPONENT  min_exponent=MP_min + MANTISSA_BITS;

}

namespace std {

template <typename M,typename E>
struct numeric_limits<::iRRAM::generic_sizetype<M,E>> {
	static constexpr bool is_specialized     = true;
	static constexpr bool is_signed          = std::numeric_limits<M>::is_signed;
	static constexpr bool is_integer         = false;
	static constexpr bool is_exact           = false;
	static constexpr bool has_infinity       = false;
	static constexpr bool has_quiet_NaN      = false;
	static constexpr bool has_signalling_NaN = false;
	static constexpr std::float_denorm_style has_denorm = std::denorm_present;
	static constexpr bool has_denorm_loss    = true; /* TODO: is this correct? */
	static constexpr std::float_round_style round_style = std::round_toward_infinity;
	static constexpr bool is_iec559          = false;
	static constexpr bool is_bounded         = true; /* TODO: should be false for verification */
	static constexpr bool is_modulo          = false;
	static constexpr int  digits             = std::numeric_limits<M>::digits - ::iRRAM::DIFF_BITS;
	static constexpr int  digits10           = digits * std::log10(2);
	static constexpr int  max_digits10       = digits10 + 2; /* TODO: is this correct? */
	static constexpr int  radix              = 2;
	static constexpr int  min_exponent       = MP_min + std::numeric_limits<M>::digits;
	// static constexpr int  min_exponent10     = ;
	static constexpr int  max_exponent       = MP_max - std::numeric_limits<M>::digits;
	// static constexpr int  max_exponent10     = ;
	static constexpr bool traps              = false;
	static constexpr bool tinyness_before    = true; /* TODO: is this correct? */

private:
	using T = ::iRRAM::generic_sizetype<M,E>;

public:
	static constexpr T min() { return { (M)1 << (digits - iRRAM::BIT_RANGE), min_exponent }; }
	static constexpr T lowest() { return { 0, min_exponent }; }
	static constexpr T max() { return { (M)1 << digits, max_exponent }; }
	static constexpr T epsilon() { return { (M)1 << (digits - iRRAM::BIT_RANGE), -(digits - iRRAM::BIT_RANGE) }; }
	// static constexpr T round_error() {}
	static constexpr T inifinity() { return min(); }
	static constexpr T quiet_NaN() { return min(); }
	static constexpr T signaling_NaN() { return min(); }
	static constexpr T denorm_min() { return { 0, min_exponent }; }
};

}

namespace iRRAM {

inline unsigned int scale(const unsigned int w,const int p) {return ((p<=GUARD_BITS)?(w>>p):0); }

// sizetype_normalize(e) **********************************************
// Try to keep the mantissa between max_mantissa and min_mantissa.
// It may not be larger than  max_mantissa.
// It still may stay smaller than min_mantissa afterwards.
// The value of e may slighty increase during this operation!
// The value of e must never decrease!
// If the resulting exponent is out of range (MP_min,MP_max)
// correct it with an increase in value or produce an error.
// ********************************************************************


inline void sizetype_normalize( sizetype& e) {
  if (iRRAM_unlikely(e.mantissa < min_mantissa)) {
      e.mantissa <<= BIT_RANGE;
      e.exponent -= BIT_RANGE;
  }
  if (iRRAM_unlikely( e.mantissa >=  max_mantissa ) ) {
      e.mantissa = ( e.mantissa>> DIFF_BITS ) + 1;
      e.exponent += DIFF_BITS;
  }
  if (iRRAM_unlikely( e.exponent < MP_min ) ){
    e.exponent = min_exponent;
  }
  if (iRRAM_unlikely( e.exponent >= MP_max ) )
  {
    iRRAM_DEBUG1(1,"exponent too big in sizetype_normalize ");
    REITERATE(0);
  }
}

inline sizetype sizetype_normalize(sizetype &&e)
{
	sizetype_normalize(e);
	return e;
}

inline sizetype sizetype_power2(int exp)
{
	return sizetype_normalize({1, exp});
}

// sizetype_inc(x,y) **************************************************
// Increment x by y
// Arguments x and y must differ!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_inc(sizetype& x,const sizetype& y)
{
  if ( y.exponent >  x.exponent) {
  x.mantissa=scale(x.mantissa,(y.exponent-x.exponent))
            +y.mantissa +1 ;
  x.exponent=y.exponent;
  } else {
  x.mantissa +=
            scale(y.mantissa,(x.exponent-y.exponent)) +1 ;
  }
  sizetype_normalize(x);
}

// x += y **************************************************
// Increment x by y
// Arguments x and y must differ!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline sizetype & operator+=(sizetype &x, const sizetype &y)
{
	sizetype_inc(x, y);
	return x;
}

// sizetype_add_one(x,y) **************************************************
// Let x = y  + 1*2^z
// Arguments x and y must differ!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_add_one(sizetype& x,const sizetype& y,const SIZETYPEEXPONENT zexp)
{
// replace 1*2^(-zexp) by 1048576*2^(-zexp-20)
// this is still the same numerical value!
// if zexp is larger than y.exponent, the result is closer to the intended result
  SIZETYPEEXPONENT zexp_scale= zexp-20;
  if ( y.exponent >  zexp_scale) {
    x.exponent= y.exponent;
    x.mantissa= y.mantissa + 1048576 ;
  } else {
    x.exponent= zexp_scale;
    x.mantissa= (scale(y.mantissa,(zexp_scale-y.exponent))+1) + 1048576 ;
  }
  sizetype_normalize(x);
}

// sizetype_inc_one(x,y) **************************************************
// Let x = x  + 1*2^z
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_inc_one(sizetype& x,const SIZETYPEEXPONENT zexp)
{
  if ( x.exponent > zexp) {
    x.mantissa ++ ;
  } else {
    x.mantissa=scale(x.mantissa,(zexp-x.exponent)) + 2 ;
    x.exponent= zexp;
  }
  sizetype_normalize(x);
}


// sizetype_inc2(x,y,z) **************************************************
// Increment x by y and z
// Arguments x, y and z must differ!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_inc2(sizetype& x,const sizetype& y,const sizetype& z)
{ SIZETYPEEXPONENT exponent;
  exponent=max3(x.exponent,y.exponent,z.exponent);
  x.mantissa=scale(x.mantissa,(exponent-x.exponent))
            +scale(y.mantissa,(exponent-y.exponent))
            +scale(z.mantissa,(exponent-z.exponent)) +2 ;
  x.exponent=exponent;
  sizetype_normalize(x);
}


// sizetype_add_wo_norm(x,y,z) **************************************************
// Add y and z yielding x
// Argument x must be different from y and z!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_add_wo_norm(sizetype& x,const sizetype& y,const sizetype& z)
{
  if ( y.exponent >  z.exponent) {
    x.exponent= y.exponent;
    x.mantissa= y.mantissa + scale(z.mantissa,(x.exponent-z.exponent))  + 1 ;
  } else {
    x.exponent= z.exponent;
    x.mantissa= z.mantissa + scale(y.mantissa,(x.exponent-y.exponent))  + 1 ;
  }
}


// sizetype_add(x,y,z) **************************************************
// Add y and z yielding x
// Argument x must be different from y and z!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_add(sizetype& x,const sizetype& y,const sizetype& z)
{
  sizetype_add_wo_norm(x,y,z);
  sizetype_normalize(x);
}

inline sizetype sizetype_add_power2(sizetype x, int exp) // TODO: better implementation possible
{
	sizetype r;
	sizetype_add(r, x, sizetype_power2(exp));
	return r;
}

// x = y + z **************************************************
// Add y and z yielding x
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline sizetype operator+(const sizetype& y,const sizetype& z)
{
  sizetype x;
  sizetype_add(x,y,z);
  return x;
}


inline void sizetype_copy(sizetype& x,const sizetype& y)
{
  x.exponent=y.exponent;
  x.mantissa=y.mantissa;
}

// sizetype_shift(x,y,s) **************************************************
// Shift y and s yielding x
// Arguments x,y may be identical!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_shift(sizetype& x,const sizetype& y,const int s)
{
  x.exponent=y.exponent+s;
  x.mantissa=y.mantissa;

  if (iRRAM_unlikely( x.exponent < MP_min ) ) {
    iRRAM_DEBUG1(1,"warning: small exponent found in sizetype_shift\n");
    x.exponent = min_exponent;
//    x.mantissa = 1;
  } else  if ( iRRAM_unlikely(x.exponent >= MP_max) )
  {
    iRRAM_DEBUG1(1,"exponent too big in sizetype_shift ");
    REITERATE(0);
  }
}

// x = y << s **************************************************
// Shift y and s yielding x
// Arguments x,y may be identical!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// *************************************************************

inline sizetype operator<<(const sizetype &y, const int s)
{
	sizetype x;
	sizetype_shift(x, y, s);
	return x;
}

// sizetype_mult(x,y,z) **************************************************
// Multiply y and z yielding x
// Argument x must be different from y and z!
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// ********************************************************************

inline void sizetype_mult(sizetype& x,const sizetype& y,const sizetype& z)
{ unsigned long long lmantissa=
     ((unsigned long long)(y.mantissa))*z.mantissa;
  x.exponent=y.exponent+z.exponent;

  while (iRRAM_unlikely(lmantissa >= max_mantissa) )
       { lmantissa=lmantissa>>BIT_RANGE2;x.exponent+=BIT_RANGE2;}

  x.mantissa=lmantissa+1;
  sizetype_normalize(x);
}

// x = y * z ***************************************************
// Multiply y and z
// The resulting value may be a bit larger than the exact value,
// The resulting value may never be smaller than the exact value
// *************************************************************

inline sizetype operator*(const sizetype &y, const sizetype &z)
{
	sizetype x;
	sizetype_mult(x, y, z);
	return x;
}

// sizetype_max(x,y,z) **************************************************
// Compute maximum of y and z in x
// Arguments x,y, and z may all be identical!
// The resulting value is exactly the maximum
// ********************************************************************

inline void sizetype_max(sizetype& x,const sizetype& y,const sizetype& z)
{ if (y.exponent>z.exponent)
  {
    if (scale(z.mantissa,y.exponent-z.exponent)>=y.mantissa) x=z; else x=y;
  } else {
    if (scale(y.mantissa,z.exponent-y.exponent)>=z.mantissa) x=y; else x=z;
  }
}

// x = max(y, z) **************************************************
// Compute maximum of y and z in x
// Arguments x,y, and z may all be identical!
// The resulting value is exactly the maximum
// ****************************************************************

inline sizetype max(const sizetype &y, const sizetype &z)
{
	if (y.exponent > z.exponent)
		return scale(z.mantissa, y.exponent-z.exponent) >= y.mantissa ? z : y;
	else
		return scale(y.mantissa, z.exponent-y.exponent) >= z.mantissa ? y : z;
}

// sizetype_set(x,m,e) **************************************************
// Construct a sizetype value x from mantissa m and exponent e
// The resulting value is allowed to be larger than the exact value
// ********************************************************************

inline void sizetype_set(sizetype& x,const int mantissa,const int exponent)
{
  x.exponent=exponent;
  x.mantissa=mantissa;
  sizetype_normalize(x);
}

inline void sizetype_exact(sizetype& x)
{
  x.exponent=min_exponent;
  x.mantissa=0;
}

/*Test whether  y<z; for y=z the result is allowed to be true OR false!  */
inline int sizetype_less(const sizetype& y,const sizetype& z)
{ unsigned int mantissa;
  if (iRRAM_unlikely(y.mantissa==0)) return 1;
  if (iRRAM_unlikely(z.mantissa==0)) return 0;
  if (y.exponent>z.exponent)
  {
    mantissa=scale(z.mantissa,y.exponent-z.exponent);
    return (mantissa>=y.mantissa);
  } else {
    mantissa=scale(y.mantissa,z.exponent-y.exponent);
    return (mantissa<z.mantissa);
  }
}

inline void sizetype_half(sizetype& x,const sizetype& y)
{
  x.exponent=y.exponent-1;
  x.mantissa=y.mantissa;
}

/************************ the following functions are unchecked for over/underflow ***************/
/* also the exact semantics has still to be defined and compared to the applications ************/


inline void sizetype_dec(sizetype& x, const sizetype& y )
{ x.mantissa=x.mantissa - scale(y.mantissa,(x.exponent-y.exponent)) -1 ;
  sizetype_normalize(x);
}

inline void sizetype_dec(sizetype& x)
{ x.mantissa -= 1 ;
}

inline void sizetype_sqrt(sizetype& x,const sizetype& y)
{
  if (y.exponent&1) {
    x.exponent=(y.exponent-1)>>1;
    x.mantissa=y.mantissa <<1;
  }  else {
    x.exponent=y.exponent>>1;
    x.mantissa=y.mantissa;
  }
  x.mantissa=int(std::sqrt(double(x.mantissa)))+1;
}

inline void sizetype_div(sizetype& x,const sizetype& y,const sizetype& z)
{ unsigned long long lmantissa=
         (((unsigned long long)(y.mantissa))<<GUARD_BITS)/z.mantissa;
  x.exponent=y.exponent-z.exponent-GUARD_BITS;

  while ( lmantissa >= max_mantissa )
       { lmantissa=lmantissa>>BIT_RANGE2;x.exponent+=BIT_RANGE2;}

  x.mantissa=lmantissa+1;
  sizetype_normalize(x);
}

}

#endif
