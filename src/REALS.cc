/*

REALS.cc -- main part of the implementation of reals for the iRRAM library
 
Copyright (C) 2001-2010 Norbert Mueller
 
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

#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <iRRAM/REAL.h>
#include <iRRAM/sizetype.hh>
#include <iRRAM/cache.h>
#include <iRRAM/SWITCHES.h>
#include <iRRAM/DYADIC.h>
#include <iRRAM/INTEGER.h>
#include <iRRAM/RATIONAL.h>

#if iRRAM_BACKEND_MPFR
# include "MPFR/MPFR_ext.h"
#else
# error "Currently no further backends defined!"
#endif

namespace iRRAM {

void REAL::mp_make_mp()
{
	if (!std::isfinite(dp.upper_neg) || !std::isfinite(dp.lower_pos))
		REITERATE(0);
	if (dp.upper_neg == -dp.lower_pos) {
		// here we have a point interval...
		if (!value)
			MP_init(value);
		MP_double_to_mp(dp.lower_pos, value);
		error = sizetype_power2(MP_min);
	} else {
		// now we know that it is not a point interval:
		MP_init(value);
		MP_double_to_mp(dp.lower_pos, value);
		MP_type value1;
		MP_init(value1);
		MP_double_to_mp(-dp.upper_neg, value1);
		MP_type value2;
		MP_init(value2);
		MP_add(value, value1, value2, -1150);
		MP_shift(value2, value, -1);
		MP_clear(value1);
		MP_clear(value2);
		// value0 differs from precise result (lower+upper)/2 by at most
		// 2^{-1200},
		// which is sufficiently more precise than all bits even of the
		// smallest double number (2^-1075), so here the error is as
		// small as reasonable
		double rwidth = (dp.upper_neg + dp.lower_pos);
		int e;
		unsigned m = (unsigned)(1073741824 * frexp(-rwidth, &e)) + 2;
		// Here, the "+2" accounts for the possible truncation error and
		// the error on the computation of cvalue. The factor 1073741824
		// is 2^30
		// So we have that the interval (dp.lower,dp.upper) is a subset
		// of the interval (cvalue - m*2^(e+29),cvalue + m*2^(e+29))
		error = sizetype_normalize({m, e - 29});
	}
	MP_getsize(value, vsize);
}

void REAL::mp_from_mp(const REAL & y)
{
	// d=MP_mp_to_double(y) is required to return a value d such that
	// there is no other double between d and x.
	// the conversion should also generate infinities in case of overflow
	// we don't require a directed rounding here!
	double center = MP_mp_to_double(y.value);
	// in consequence, wd will be the negative of an upper bound
	// for the width of the interval!
	double wd = ldexp(-double(y.error.mantissa), y.error.exponent);
	dp.upper_neg = nextafter(-center, -INFINITY) + wd;
	dp.lower_pos = nextafter(center, -INFINITY) + wd;
}

void REAL::mp_from_int(const int i)
{
	MP_init(value);
	MP_int_to_mp(i, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

void REAL::mp_from_double(const double d)
{
	MP_init(value);
	MP_double_to_mp(d, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

void REAL::mp_copy(const REAL & y)
{
	MP_duplicate_wo_init(y.value, value);
	error = y.error;
	vsize = y.vsize;
}

void REAL::mp_copy_init(const REAL & y)
{
	MP_duplicate_w_init(y.value, value);
	error = y.error;
	vsize = y.vsize;
}

REAL REAL::mp_addition(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max({y.error.exponent, this->error.exponent,
		                  state.ACTUAL_STACK.actual_prec});
	else {
		local_prec = max(this->vsize.exponent, y.vsize.exponent);
		local_prec = max({y.error.exponent, this->error.exponent,
		                  local_prec - 50 + state.ACTUAL_STACK.actual_prec});
	}
	MP_init(zvalue);
	MP_mv_add(this->value, y.value, zvalue, local_prec);

	sizetype_add_wo_norm(zerror, this->error, y.error);
	sizetype_inc_one(zerror, local_prec);

	return REAL(zvalue, zerror);
}

REAL REAL::mp_addition(const int n) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec = 0;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(this->error.exponent, state.ACTUAL_STACK.actual_prec);
	else {
		sizetype ysize;
		ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
		local_prec = max(this->vsize.exponent, ysize.exponent);
		local_prec = max(this->error.exponent,
		                 local_prec - 50 + state.ACTUAL_STACK.actual_prec);
	}
	MP_init(zvalue);
	MP_mv_addi(this->value, n, zvalue, local_prec);
	sizetype_add_one(zerror, this->error, local_prec);
	return REAL(zvalue, zerror);
}

REAL & REAL::mp_eqaddition(const REAL & y)
{
	MP_type zvalue;
	int local_prec;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max({y.error.exponent, this->error.exponent,
		                  state.ACTUAL_STACK.actual_prec});
	else {
		local_prec = max(this->vsize.exponent, y.vsize.exponent);
		local_prec = max({y.error.exponent, this->error.exponent,
		                  local_prec - 50 + state.ACTUAL_STACK.actual_prec});
	}
	MP_init(zvalue);
	MP_mv_add(this->value, y.value, zvalue, local_prec);

	this->error += y.error;
	sizetype_inc_one(this->error, local_prec);

	/*  zerror = sizetype_power2(local_prec);
	  sizetype_inc2(this->error,y.error,zerror);*/

	MP_clear(this->value);
	this->value = zvalue;
	MP_getsize(this->value, this->vsize);
	return (*this);
}

std::string swrite(const REAL & x, const int w, const float_form form)
{
	if (!x.value) {
		REAL y(x);
		y.mp_make_mp();
		return swrite(y, w, form);
	}

	std::string result;
	if (state.ACTUAL_STACK.inlimit == 0 &&
	    state.thread_data_address->cache_s.get(result))
		return result;

	int width = w;
	if (width < 9)
		width = 9;
	char * erg;

	switch (form) {
	case float_form::absolute: {
		sizetype psize;
		int p = -10 * (width - 8) / 3;
		psize = sizetype_power2(p);
		int s = MP_size(x.value);
		int mantissa =
		        (int)((s - x.error.exponent - GUARD_BITS) * .30103);
		if (sizetype_less(psize, x.error) ||
		    (s > p && mantissa + 8 < width)) {
			iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in "
			                "conversion with precision 2^(%d)\n",
			             x.error.mantissa, x.error.exponent, p);
			REITERATE(p - x.error.exponent);
		}

		if (s <= p) {
			erg = (char *)calloc(width + 1, sizeof(char));
			strcpy(erg, "  0 ");
			for (int j = 4; j < width; j++)
				erg[j] = ' ';
			erg[width] = 0;
		} else {
			erg = MP_swrite(x.value, width);
		}
		break;
	}
	case float_form::relative: {
		int p;
		{
			single_valued code;
			// We want to have an error less than 0.65 ulp, where
			// 0.51 ulp come from
			// the conversion to decimal, so we need to know that
			// the error is at most
			// 0.15ulp.
			// At the time, we take the size of x....
			// It would be better to take the x.size, subtract
			// x.error and approximately
			// convert the result to decimal in order to get a
			// correct approximation
			// for a lower bound of x. Using this lower bound we can
			// estimate what
			// 0.15ulp means in binary.
			p = size(x) - 2 - 10 * (width - 8) / 3;
			// We have to prevent that "size" changes the
			// multi-value-cache here!
			// Otherwise, any iteration would not use the same
			// amount of
			// cache as the first run!
		}
		sizetype psize;
		psize = sizetype_power2(p);
		int s = MP_size(x.value);
		int mantissa =
		        (int)((s - x.error.exponent - GUARD_BITS) * .30103);
		if (sizetype_less(psize, x.error) ||
		    (s > p && mantissa + 8 < width)) {
			iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in "
			                "writing with precision 2^(%d)\n",
			             x.error.mantissa, x.error.exponent, p);
			REITERATE(p - x.error.exponent);
		}
		erg = MP_swrite(x.value, width);
		break;
	}
	case float_form::show: {
		int mantissa = (int)(int((MP_size(x.value) - x.error.exponent -
		                          GUARD_BITS)) *
		                     .30103);
		if (mantissa < 1) {
			erg = (char *)calloc(width + 1, sizeof(char));
			sprintf(erg, " .*E%05.0f",
			        1 + (x.error.exponent + GUARD_BITS) * .30103);
			for (int j = 9; j < width; j++)
				erg[j] = ' ';
		} else {
			if (mantissa > width - 8)
				mantissa = width - 8;
			char * erg2 = MP_swrite(x.value, mantissa + 8);
			erg = (char *)calloc(width + 1, sizeof(char));
			strncpy(erg, erg2, mantissa + 8);
			for (int j = mantissa + 8; j < width; j++)
				erg[j] = ' ';
			erg[width] = 0;
			free(erg2);
		}
		break;
	}
	}

	result = erg;
	free(erg);
	if (state.ACTUAL_STACK.inlimit == 0)
		state.thread_data_address->cache_s.put(result);
	return result;
}

REAL REAL::mp_subtraction(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max({y.error.exponent, this->error.exponent,
		                  state.ACTUAL_STACK.actual_prec});
	else {
		local_prec = max(this->vsize.exponent, y.vsize.exponent);
		local_prec = max({y.error.exponent, this->error.exponent,
		                  local_prec - 50 + state.ACTUAL_STACK.actual_prec});
	}
	MP_init(zvalue);
	MP_mv_sub(this->value, y.value, zvalue, local_prec);

	sizetype_add_wo_norm(zerror, this->error, y.error);
	sizetype_inc_one(zerror, local_prec);
	/*
	  zerror = sizetype_power2(local_prec);
	  sizetype_inc2(zerror,this->error,y.error);
	*/
	return REAL(zvalue, zerror);
}

REAL REAL::mp_subtraction(const int n) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec =
		        max(this->error.exponent, state.ACTUAL_STACK.actual_prec);
	else {
		sizetype ysize;
		ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
		local_prec = max(this->vsize.exponent, ysize.exponent);
		local_prec = max(this->error.exponent,
		                 local_prec - 50 + state.ACTUAL_STACK.actual_prec);
	}
	MP_init(zvalue);
	MP_mv_subi(this->value, n, zvalue, local_prec);
	sizetype_add_one(zerror, this->error, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_invsubtraction(const int n) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(this->error.exponent, state.ACTUAL_STACK.actual_prec);
	else {
		sizetype xsize, ysize;
		MP_getsize(this->value, xsize);
		ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
		local_prec = max(xsize.exponent, ysize.exponent);
		local_prec = max(this->error.exponent,
		                 local_prec - 50 + state.ACTUAL_STACK.actual_prec);
	}
	MP_init(zvalue);
	MP_mv_isub(n, this->value, zvalue, local_prec);
	sizetype_add_one(zerror, this->error, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_multiplication(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror, proderror, sumerror;
	int local_prec;
	zerror = this->vsize * y.error;
	sizetype_add_wo_norm(sumerror, y.vsize, y.error);
	proderror = sumerror * this->error;
	zerror += proderror;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(zerror.exponent, state.ACTUAL_STACK.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent + y.vsize.exponent - 50 +
		                         state.ACTUAL_STACK.actual_prec);
	MP_init(zvalue);
	MP_mv_mul(this->value, y.value, zvalue, local_prec);
	sizetype_inc_one(zerror, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_multiplication(const int n) const
{
	MP_type zvalue;
	sizetype zerror, ysize;
	int local_prec;
	ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
	zerror = ysize * this->error;

	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(zerror.exponent, state.ACTUAL_STACK.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent + ysize.exponent - 50 +
		                         state.ACTUAL_STACK.actual_prec);
	MP_init(zvalue);
	MP_mv_muli(this->value, n, zvalue, local_prec);
	sizetype_inc_one(zerror, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_division(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror, h1, h2, h3;
	int local_prec;
	sizetype_half(h1, y.vsize);
	if (sizetype_less(h1, y.error)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in "
		                "denominator of size %d*2^(%d)\n",
		             y.error.mantissa, y.error.exponent,
		             y.vsize.mantissa, y.vsize.exponent);
		REITERATE(0);
	}
	h1 = this->vsize * y.error;
	h2 = y.vsize * this->error;
	h1 += h2;
	h3 = y.vsize;
	sizetype_dec(h3);
	sizetype_dec(h3, y.error);
	h2 = h3 * y.vsize;
	sizetype_div(zerror, h1, h2);
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(zerror.exponent, state.ACTUAL_STACK.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent - y.vsize.exponent - 50 +
		                         state.ACTUAL_STACK.actual_prec);
	MP_init(zvalue);
	MP_mv_div(this->value, y.value, zvalue, local_prec);
	sizetype_inc_one(zerror, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_division(const int n) const
{
	MP_type zvalue;
	sizetype zerror, ysize;
	int local_prec;
	ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
	sizetype_div(zerror, this->error, ysize);
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(zerror.exponent, state.ACTUAL_STACK.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent - ysize.exponent - 50 +
		                         state.ACTUAL_STACK.actual_prec);
	MP_init(zvalue);
	MP_mv_divi(this->value, n, zvalue, local_prec);
	sizetype_inc_one(zerror, local_prec);
	return REAL(zvalue, zerror);
}


void rwrite(const REAL & x, const int w)  { cout << swrite(x, w, float_form::absolute); }
void rwritee(const REAL & x, const int w) { cout << swrite(x, w, float_form::relative); }
void rshow(const REAL & x, const int w)   { cout << swrite(x, w, float_form::show); }


REAL REAL::mp_square() const
{
	MP_type zvalue;
	sizetype zerror, proderror;
	int local_prec;
	zerror = this->vsize * this->error;
	proderror = this->vsize * this->error;
	zerror += proderror;
	proderror = this->error * this->error;
	zerror += proderror;
	if (state.ACTUAL_STACK.prec_policy == 0)
		local_prec = max(zerror.exponent, state.ACTUAL_STACK.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent + this->vsize.exponent -
		                         50 + state.ACTUAL_STACK.actual_prec);
	MP_init(zvalue);
	MP_mv_mul(this->value, this->value, zvalue, local_prec);
	sizetype_inc_one(zerror, local_prec);
	return REAL(zvalue, zerror);
}

LAZY_BOOLEAN REAL::mp_less(const REAL & y) const
{
	REAL z = y - (*this);
	sizetype s;
	MP_getsize(z.value, s);
	if (sizetype_less(s, z.error)) {
		iRRAM_DEBUG2(1, "insufficient precisions %d*2^(%d) and "
		                "%d*2^(%d) in comparing\n",
		             this->error.mantissa, this->error.exponent,
		             y.error.mantissa, y.error.exponent);
		return LAZY_BOOLEAN::BOTTOM;
	}
	return ((MP_sign((z.value)) == 1));
}

REAL REAL::mp_absval() const
{
	MP_type zvalue;
	MP_init(zvalue);
	MP_abs(value, zvalue);
	return REAL(zvalue, error);
}

// REAL REAL::mp_interval_join (const REAL& y)const
// {
// /* The purpose of this routine is to compute interval hull (as a simplified
//  * interval) to the union of two intervals (where both are also given a
//  * simplified intervals). */
// 
// error("this is unfinished!")
// 
//   MP_type zvalue; sizetype zerror; int local_prec;
//   if (ACTUAL_STACK.prec_policy == 0)
//   local_prec=max({y.error.exponent,this->error.exponent,ACTUAL_STACK.actual_prec});
//   else {
//   local_prec=max(this->vsize.exponent,y.vsize.exponent);
//   local_prec=max({y.error.exponent,this->error.exponent,local_prec-50+ACTUAL_STACK.actual_prec});
//   }
//   MP_init(zvalue);
//   MP_mv_add(this->value,y.value,zvalue,local_prec);
// 
//   sizetype_add_wo_norm(zerror,this->error,y.error);
//   sizetype_inc_one(zerror,local_prec);
// 
//   return REAL(zvalue,zerror);
// }


/*****************************************************/

REAL scale(const REAL & x, int n)
{
	if (!x.value) {
		/* TODO: huh? why not x.mp_conv()? For instance
		 * operator+(const REAL &, const REAL &) does the same */
		REAL y(x);
		return scale(y.mp_conv(), n);
	}
	sizetype zerror;
	MP_type zvalue;
	MP_init(zvalue);
	MP_shift(x.value, zvalue, n);
	x.geterror(zerror);
	zerror = zerror << n;
	return REAL(zvalue, zerror);
}


LAZY_BOOLEAN positive(const REAL & x, int k)
{
	if (!x.value) {
		REAL y(x);
		return positive(y.mp_conv(), k);
	}
	bool erg;
	sizetype ksize;
	ksize = sizetype_power2(k);
	if (sizetype_less(ksize, x.error) && sizetype_less(x.vsize, x.error)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in test on "
		                "positive\n",
		             x.error.mantissa, x.error.exponent);
		return LAZY_BOOLEAN::BOTTOM;
	}
	erg = (MP_sign(x.value) == 1);
	return erg;
}

DYADIC approx(const REAL & x, const int p)
{
	if (!x.value) {
		REAL y(x);
		return approx(y.mp_conv(), p);
	}
	MP_type result;
	MP_type erg;
	if ((state.ACTUAL_STACK.inlimit == 0) &&
	    state.thread_data_address->cache_mp.get(result)) {
		MP_duplicate_w_init(result, erg);
		return DYADIC(erg);
	}

	sizetype psize;
	sizetype_set(psize, 1, p + 1);
	if (sizetype_less(psize, x.error)) {
		iRRAM_DEBUG2(1,
		             "insufficient precision %d*2^(%d) in approx(%d)\n",
		             x.error.mantissa, x.error.exponent, p);
		REITERATE(p - x.error.exponent);
	}
	MP_init(erg);
	MP_copy(x.value, erg, p - 1);

	if (state.ACTUAL_STACK.inlimit == 0) {
		MP_duplicate_w_init(erg, result);
		state.thread_data_address->cache_mp.put(result);
	}
	return DYADIC(erg);
}

int size(const REAL & x)
{
	if (!x.value) {
		REAL y(x);
		return size(y.mp_conv());
	}
	int result = 0;
	if ((state.ACTUAL_STACK.inlimit == 0) &&
	    state.thread_data_address->cache_i.get(result))
		return result;
	sizetype xsize = x.vsize;
	sizetype ergsize;
	sizetype_add(ergsize, xsize, x.error);
	unsigned int value = ergsize.mantissa;
	for (unsigned int m = 32 >> 1; m > 0; m = m >> 1) {
		if (value >= (((unsigned int)1) << m)) {
			result += m;
			value = value >> m;
		}
	}
	if (((unsigned int)1 << result) != ergsize.mantissa)
		result += 1;
	result += ergsize.exponent;

	if (result < MP_min)
		throw iRRAM_Numerical_Exception(iRRAM_underflow_error);

	sizetype_set(xsize, 1, result - 2);
	sizetype_inc(xsize, x.error);
	if (sizetype_less(x.vsize, xsize)) {
		iRRAM_DEBUG2(
		        1,
		        "insufficient precision %d*2^(%d) in size %d*2^(%d)\n",
		        x.error.mantissa, x.error.exponent, x.vsize.mantissa,
		        x.vsize.exponent);
		REITERATE(0);
	}

	if (state.ACTUAL_STACK.inlimit == 0)
		state.thread_data_address->cache_i.put(result);
	return result;
}

int upperbound(const REAL & x)
{
	if (!x.value) {
		REAL y(x);
		return upperbound(y.mp_conv());
	}
	int result;
	sizetype ergsize;
	if ((state.ACTUAL_STACK.inlimit == 0) &&
	    state.thread_data_address->cache_i.get(result))
		return result;
	sizetype_add(ergsize, x.vsize, x.error);
	while (ergsize.mantissa > (1 << 16)) {
		ergsize.mantissa >>= 16;
		ergsize.exponent += 16;
	}
	while (ergsize.mantissa > (1 << 4)) {
		ergsize.mantissa >>= 4;
		ergsize.exponent += 4;
	}
	while (ergsize.mantissa > 1) {
		ergsize.mantissa >>= 1;
		ergsize.exponent += 1;
	}
	result = ergsize.exponent;
	if (state.ACTUAL_STACK.inlimit == 0)
		state.thread_data_address->cache_i.put(result);
	return result;
}

LAZY_BOOLEAN bound(const REAL & x, const int k)
{
	if (!x.value) {
		REAL y(x);
		return bound(y.mp_conv(), k);
	}
	sizetype lowsize, ksize, highsize;
	sizetype_set(ksize, 1, k);
	sizetype_add_one(lowsize, x.error, k - 1);
	sizetype_add(highsize, x.vsize, x.error);
	if (sizetype_less(x.vsize, lowsize) && sizetype_less(ksize, highsize)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in bounding "
		                "by 2^(%d) for argument of size  %d*2^(%d)\n",
		             x.error.mantissa, x.error.exponent, k,
		             x.vsize.mantissa, x.vsize.exponent);
		return LAZY_BOOLEAN::BOTTOM;
	}
	return (sizetype_less(x.vsize, ksize));
}


/*! \ingroup debug */
void REAL::rcheck(int n) const
{
	if (!value) {
		cerr << "Value: (" << std::setprecision(n) << std::setw(n)
		     << dp.lower_pos << ";" << std::setprecision(n)
		     << std::setw(n) << -dp.upper_neg << ")\n";
	} else {
		cerr << "Value: ";
		char * c = MP_swrite(value, n);
		cerr << c;
		free(c);
		cerr << ", Size:  " << vsize.mantissa << "*2^("
		     << vsize.exponent << ")\n\n";
		cerr << ", Error: " << error.mantissa << "*2^("
		     << error.exponent << ")\n\n";
	}
}

void REAL::adderror(sizetype nerror)
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().adderror(nerror);
		*this = y;
	} else {
		sizetype_inc((*this).error, nerror);
	}
}

void REAL::seterror(sizetype nerror)
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().seterror(nerror);
		*this = y;
	} else {
		(*this).error = nerror;
	}
}

void REAL::geterror(sizetype & nerror) const
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().geterror(nerror);
	} else {
		nerror = (*this).error;
	}
}

void REAL::getsize(sizetype & nsize) const
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().getsize(nsize);
	} else {
		nsize = (*this).vsize;
	}
}

void REAL::to_formal_ball(DYADIC & d, sizetype & nerror) const
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().geterror(nerror);
		MP_duplicate_wo_init(y.value, d.value);
	} else {
		nerror = (*this).error;
		MP_duplicate_wo_init(this->value, d.value);
	}
}

REAL::REAL(const DYADIC & y)
{
	MP_duplicate_w_init(y.value, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

REAL::REAL(const std::string &s) : REAL(s.c_str()) {}
REAL::REAL(const char        *s) : REAL(atoREAL(s)) {}

INTEGER REAL::as_INTEGER() const
{
	if (!this->value) {
		return this->mp_conv().as_INTEGER();
	}
	MP_int_type result, value;
	if ((state.ACTUAL_STACK.inlimit == 0) &&
	    state.thread_data_address->cache_mpi.get(result)) {
		MP_int_duplicate_w_init(result, value);
		return value;
	}

	sizetype psize;
	sizetype_set(psize, 1, -4);
	REAL y = (*this);
	if (MP_sign(y.value) == 1)
		y = y + REAL(0.5);
	else
		y = y - REAL(0.5);
	y.mp_conv();
	if (sizetype_less(psize, y.error)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) converting "
		                "to integer\n",
		             this->error.mantissa, this->error.exponent);
		REITERATE(-y.error.exponent);
	}
	MP_int_init(value);
	MP_mp_to_INTEGER(y.value, value);

	if (state.ACTUAL_STACK.inlimit == 0) {
		MP_int_duplicate_w_init(value, result);
		state.thread_data_address->cache_mpi.put(result);
	}
	return value;
}

// conversion to type REAL from smaller types

REAL::REAL(const RATIONAL & r)
{
	INTEGER numi, deni;

	MP_rat_get_numerator(r.value, numi.value);
	MP_rat_get_denominator(r.value, deni.value);

	REAL result = REAL(numi) / REAL(deni);
	result.mp_conv();
	error = result.error;
	MP_duplicate_w_init(result.value, value);
	MP_getsize(value, vsize);
}

REAL::REAL(const INTEGER & y)
{
	MP_init(value);
	MP_INTEGER_to_mp(y.value, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

REAL strtoREAL2(const char *s, char **endptr)
{
	const char *t = s;
	const char *int_start, *int_end;
	const char *frac_start, *frac_end;
	int frac_zeroes = 0;

	while (isblank(*t)) t++;
	if (*t == '+' || *t == '-') t++;
	while (*t == '0') t++;
	int_start = t;
	while ('0' <= *t && *t <= '9') t++;
	int_end = t;

	if (*t == '.') t++;
	frac_start = t;
	while (*t == '0') { frac_zeroes++; t++; }
	while ('0' <= *t && *t <= '9') t++;
	frac_end = t;
	while (frac_end > frac_start && frac_end[-1] == '0') frac_end--;

	long exp = 0;
	if (*t == 'e' || *t == 'E')
		exp = std::strtol(t+1, endptr, 10);

	/* d[k] ... d[0] . d[-1] .. d[-n] */

	stiff code;

	int k = (int)(int_end - int_start) - 1;
	int n = frac_end - frac_start;
	int local_prec = state.ACTUAL_STACK.actual_prec;
	int q = ((k + 1 - (k<0?frac_zeroes:0) + exp)*10+2)/3 - local_prec;

	MP_type value;
	MP_init(value);
	mpfr_set_prec(value, max(10, q+1));
	int r = mpfr_strtofr(value, s, endptr, 10, MPFR_RNDN);
	ext_mpfr_remove_trailing_zeroes(value);

	return REAL(value, r ? sizetype_power2(local_prec) : sizetype_exact());
}

REAL strtoREAL(const char* s, char** endptr){
  stiff code;
  int exp=0;
  int sign=1;
  REAL y;
  REAL ten=10;
  *endptr=const_cast<char*> (s);

  while ( **endptr == '0' ) *endptr+=1;

  if ( **endptr == '-' ) {sign=-1;*endptr+=1;}
  else if ( **endptr == '+' ) {sign=1;*endptr+=1;}

  while ( **endptr >= '0' &&  **endptr <= '9' ) {
  y=ten*y + REAL(sign*(**endptr-'0')); *endptr+=1;}

  if ( **endptr == '.' ) *endptr+=1;

  while ( **endptr >= '0' &&  **endptr <= '9' ) {
  y=ten*y + REAL(sign*(**endptr-'0')); exp-=1;*endptr+=1;}

  if ( **endptr == 'E' || **endptr == 'e'  ) {
  *endptr+=1;exp+=strtol(*endptr,endptr,10); }

  if (exp !=0) y*=power(ten,exp);

  return y;
}

REAL atoREAL(const char* s){
  char*dummy;
  return strtoREAL(s,&dummy);
}


REAL modulo (const REAL& x, const REAL& y){
   return x-round2(x/y)*y;
}

REAL power(const REAL& x, const REAL& y) {
  return exp(log(x)*y);
}


REAL power(const REAL& x, int n) {
   if (n==0) return 1;
   if (n==1) return x;
   if (n==2) return square(x);
//   stiff_begin();
   REAL y=1;
   REAL xc=x;
   if (n<0) {xc=y/x;n=-n;}
   if (n==1) {
//     stiff_end();
     return xc;
   }
   for (int k=n;k>0;k=k/2) { 
     if (k%2==1) y*=xc;
     if ( k ==1) break;
     xc=square(xc);}
//   stiff_end();
   return y; 
}

// maximum without using of internal representation of LAZY_BOOLEAN
// REAL maximum (const REAL& x, const REAL& y){
//    single_valued code;
//    LAZY_BOOLEAN larger = ( x > y );
//    switch ( choose ( larger, !larger, TRUE ) ){
//    case 1: return x;
//    case 2: return y;
//    case 3: return (x+y+abs(x-y))/2;
//    }
// };

// maximum, using of internal representation of LAZY_BOOLEAN
REAL maximum (const REAL& x, const REAL& y){
   LAZY_BOOLEAN larger;
   {
     single_valued code;
     larger = ( x > y );
   }
   if ( larger.value == true  ) return x;
   if ( larger.value == false ) return y;
   return (x+y+abs(x-y))/2;
}

// minimum, using of internal representation of LAZY_BOOLEAN
REAL minimum (const REAL& x, const REAL& y){
   LAZY_BOOLEAN larger;
   {
     single_valued code;
     larger = ( x > y );
   }
   if ( larger.value  == true  ) return y;
   if ( larger.value  == false ) return x;
   return (x+y-abs(x-y))/2;
}

//********************************************************************************
// Absolute value of vector in Euclidean space
//********************************************************************************

REAL abs(const std::vector<REAL>& x)
{
	unsigned int n=x.size();
	REAL sqrsum=0;
	for (unsigned i=0;i<n;i++) {
		sqrsum += square(x[i]);
	}
	return sqrt(sqrsum);
}

// Conversions from REAL to DYADIC with absolute precision
DYADIC REAL::as_DYADIC(const int p) const { return approx(*this, p); }

DYADIC REAL::as_DYADIC() const { return approx(*this, DYADIC::getprec()); }

// Conversion from REAL to double with a relative precision of p bits
double REAL::as_double(const int p) const
{
	if (bound(*this, -1150))
		return 0.0;
	int s = size(*this);
	DYADIC d = approx(*this, s - p - 2);
	return MP_mp_to_double(d.value);
}

/*****************************************/
// module function (will be a template later...)

int module(REAL f(const REAL&),const REAL& x, int p){
// Semantics: If m=module(f,x,p), then
//    |x-z| <=2^m implies  |f(x)-f(z)| <= 2^p 

// If we are able to approximate f(x) by a DYADIC number d with an error of <=2^{p-1},
// then for any z in the argument interval of x, f(z) must differ by at most 
//  2^{p-1} from d, hence |f(x)-f(z)|<=2^p

  int result;
  if ( (state.ACTUAL_STACK.inlimit==0) && state.thread_data_address->cache_i.get(result)) return result;

  DYADIC d;
  REAL x_copy=x;

  
  sizetype argerror,testerror;
  
  x_copy.geterror(argerror);
  testerror = sizetype_power2(argerror.exponent);
  x_copy.adderror(testerror);
  {
    single_valued code;
    d=approx(f(x_copy),p-1); 
  }
// At this line, we are sure that x_copy (and so also x) is precise enough to allow 
// the computation of f(x), even with a slightly increased error of the argument.

// We now try to find the smallest p_arg such that the evaluation of f(x+- 2^p_arg) 
// is possible up to an error of at most  2^{p-1}

// To do this, we start with p_arg=p.
// If this is successfull, we increase the value of p_arg until the first failure
// It it is not, then we decrease until the first success...
  int direction=0,p_arg=p;
  bool try_it=true;

  while (try_it) {
    testerror = sizetype_power2(p_arg);
    x_copy.seterror(argerror);
    x_copy.adderror(testerror);
    bool fail = false;
  if ( iRRAM_unlikely(state.debug > 0 ) ) {
   sizetype x_error;
   x_copy.geterror(x_error);
  iRRAM_DEBUG2(1,"Testing module: 1*2^%d + %d*2^%d\n",p_arg,argerror.mantissa,argerror.exponent);
  iRRAM_DEBUG2(1,"argument error: %d*2^%d\n",x_error.mantissa,x_error.exponent);
  }
  try { 
      single_valued code;
      REAL z=f(x_copy);
      if ( iRRAM_unlikely(state.debug > 0 ) ) {
        sizetype z_error;
        z.geterror(z_error);
        iRRAM_DEBUG2(1,"Module yields result %d*2^%d\n",z_error.mantissa,z_error.exponent);
      }
      d=approx(z,p-1); 
	}
    catch ( Iteration it)  { fail=true; }
    switch ( direction ) {
      case 0:;
        if ( fail ) direction=-1; else direction=1;
	p_arg+=direction;
      break;
      case 1:;
        if ( fail ) { try_it=false; p_arg -=direction; }
	else { p_arg += direction; };
      break;
      case -1:;
        if ( fail ) { p_arg +=direction; }
	else { try_it=false; };
      break;    
    }
    }
  iRRAM_DEBUG2(1,"Modules resulting in p_arg=%d\n",p_arg);
  
  testerror = sizetype_power2(p_arg);
  argerror += testerror;

  while (argerror.mantissa>1) {
    argerror.mantissa=argerror.mantissa>>1;
    argerror.exponent+=1;
  }
  
  result=argerror.exponent;
  if ( state.ACTUAL_STACK.inlimit==0 ) state.thread_data_address->cache_i.put(result);
  return result;

}

} // namespace iRRAM
