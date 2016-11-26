/*

stack.cc -- basic file for the iterations in the iRRAM library
 
Copyright (C) 2001-2006 Norbert Mueller
 
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

#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <iRRAM/core.h>

//*************************************************************************************
// runtime identification of the iRRAM version, cf. iRRAM_version.h
//*************************************************************************************

extern "C" {
const char *iRRAM_VERSION_rt = iRRAM_VERSION_ct;
}

namespace iRRAM {

// iRRAM_TLS bool iRRAM_COMPARE_exact=true; /* unused */
// iRRAM_TLS int iRRAM_COMPARE_precision=-60; /* unused */

iRRAM_TLS state_t state;

iRRAM_thread_data_class::iRRAM_thread_data_class() = default;
iRRAM_thread_data_class::~iRRAM_thread_data_class() = default;

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
  testerror = sizetype_normalize({1,argerror.exponent});
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
    testerror = sizetype_normalize({1,p_arg});
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
  
  testerror = sizetype_normalize({1,p_arg});
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
// 
