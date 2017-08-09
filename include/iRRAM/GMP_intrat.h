/*

GMP_intrat.h -- extension to the GMP integer/rationals for the iRRAM library
 
Copyright (C) 2001-2004 Norbert Mueller
 
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

/*
Changelog: (initial version by Tom van Diessen)  

  2001-09-10 removal of several unneeded functions by Norbert
  2004-12-20 transformation to inline by Norbert
*/

#ifndef iRRAM_GMP_INT_RAT_H
#define iRRAM_GMP_INT_RAT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gmp.h>

#include <iRRAM/version.h>

#ifdef __cplusplus
extern "C" {
#endif

/*GMP_min and GMP_max should go to the interface definitions!*/

#define GMP_min -1000000000
#define GMP_max 1000000000

/********** counting vars **********/
#define MaxFreeVars 1000
extern iRRAM_TLS mpz_ptr gmp_FreeVarsi[MaxFreeVars];
extern iRRAM_TLS int gmp_FreeVarCounti;
extern iRRAM_TLS int int_gmp_var_count;


/********** initialization function **********/

static inline mpz_ptr int_gmp_init()
{
	mpz_ptr z;
	if (gmp_FreeVarCounti>0) {
		gmp_FreeVarCounti--;
		z = gmp_FreeVarsi[gmp_FreeVarCounti];
	}else{
		z = (mpz_ptr) malloc(sizeof(mpz_t));
		mpz_init(z);
	}
	int_gmp_var_count++;
	return z;
}

static inline void int_gmp_free(mpz_ptr z)
{
  if (gmp_FreeVarCounti < MaxFreeVars){
  		gmp_FreeVarsi[gmp_FreeVarCounti]=z;
  		gmp_FreeVarCounti++;
  }else{
  	mpz_clear(z);
  	free(z);
  }
  int_gmp_var_count--;
}

/********** Conversion functions **********/

static inline void int_gmp_int2integer(const int i, mpz_t z){mpz_set_si(z,i);}
static inline void int_gmp_double2int(const double d, mpz_t z){mpz_set_d(z,d);}
static inline void int_gmp_string2int(const char* s, mpz_t z, int b){mpz_set_str(z,s,b);}
static inline int int_gmp_integer2int(const mpz_t z){return mpz_get_si(z);}

/********** standard arithmetic functions for MP integer **********/

static inline void int_gmp_add(const mpz_t z1, const mpz_t z2, mpz_t z){mpz_add(z,z1,z2);}
static inline void int_gmp_sub(const mpz_t z1, const mpz_t z2, mpz_t z){mpz_sub(z,z1,z2);}
static inline void int_gmp_mul(const mpz_t z1, const mpz_t z2, mpz_t z){mpz_mul(z,z1,z2);}
static inline void int_gmp_div(const mpz_t z1, const mpz_t z2, mpz_t z){mpz_tdiv_q(z,z1,z2);}

static inline void int_gmp_add_ui(const mpz_t z1, const unsigned int z2, mpz_t z){mpz_add_ui(z,z1,z2);}
static inline void int_gmp_sub_ui(const mpz_t z1, const unsigned int z2,mpz_t z){mpz_sub_ui(z,z1,z2);}
static inline void int_gmp_mul_si(const mpz_t z1, const int z2, mpz_t z){mpz_mul_si(z,z1,z2);}
static inline void int_gmp_div_ui(const mpz_t z1, const unsigned int z2, mpz_t z){mpz_tdiv_q_ui(z,z1,z2);}

static inline void int_gmp_abs(const mpz_t z1, mpz_t z){mpz_abs(z, z1);}
static inline void int_gmp_neg(const mpz_t z1, mpz_t z){mpz_neg(z, z1);}



/********** more MP integer functions *********/

static inline void int_gmp_root(const mpz_t z1, unsigned int z2, mpz_t z){mpz_root(z, z1, z2);}
static inline void int_gmp_power_i(const mpz_t z1, unsigned int z2, mpz_t z){mpz_pow_ui(z, z1, z2);}
static inline void int_gmp_power_ii(unsigned int z1, unsigned int z2, mpz_t z){mpz_ui_pow_ui(z, z1, z2);}
static inline void int_gmp_fac(unsigned int z1, mpz_t z){mpz_fac_ui(z, z1);}
static inline void int_gmp_modulo(const mpz_t z1, const mpz_t z2, mpz_t z){mpz_mod(z, z1, z2);}
static inline void int_gmp_sqrt(const mpz_t z1, mpz_t z){mpz_sqrt(z,z1);}
void int_gmp_shift(const mpz_t z1, mpz_t z, int p);
//int int_gmp_log(mpz_t z);


/********** output functions for MP integers **********/

//void int_gmp_writee(const mpz_t z, int w);
char* int_gmp_swritee(const mpz_t z, int w);
//void int_gmp_write(const mpz_t z, int w);
//void int_gmp_outstr(const mpz_t z, int w);
//void int_gmp_printf(const mpz_t z);
static inline char* int_gmp_sprintf(const mpz_t z){return mpz_get_str(NULL,10,z);}


/********** copying MP integers with/without initializing **********/

static inline void int_gmp_duplicate_w_init(const mpz_t z1, mpz_ptr *z2){
  *z2=int_gmp_init();
  mpz_set(*z2,z1);
}

static inline void int_gmp_duplicate_wo_init(const mpz_t z1, mpz_t z2){mpz_set(z2,z1);}

/********* sign, size, and comparison of integer*/
static inline int int_gmp_sgn(const mpz_t z){return mpz_sgn(z);}
static inline int int_gmp_cmp(const mpz_t z1,const  mpz_t z2){return mpz_cmp(z1,z2);}
static inline int int_gmp_size(const mpz_t z){
	if (mpz_sgn(z) == 0 )  return 0; else   return mpz_sizeinbase(z,2);}

/********** counting vars **********/
#define rat_MaxFreeVars 1000
extern iRRAM_TLS int rat_gmp_var_count;
extern iRRAM_TLS mpq_ptr rat_gmp_FreeVarsi[rat_MaxFreeVars];
extern iRRAM_TLS int rat_gmp_FreeVarCount;

/********** initialization function **********/

static inline mpq_ptr rat_gmp_init(){
	mpq_ptr z;
	if (rat_gmp_FreeVarCount>0){
		rat_gmp_FreeVarCount--;
		z = rat_gmp_FreeVarsi[rat_gmp_FreeVarCount];
	}else{
		z = (mpq_ptr) malloc(sizeof(mpq_t));
		mpq_init(z);
	}
	rat_gmp_var_count++;
	return z;
}

static inline void rat_gmp_free(mpq_ptr z){
	if(rat_gmp_FreeVarCount < rat_MaxFreeVars){
		rat_gmp_FreeVarsi[rat_gmp_FreeVarCount]=z;
		rat_gmp_FreeVarCount++;
	}else{
		mpq_clear(z);
		free(z);
	}
	rat_gmp_var_count--;
}

/* canonicalize the given rational, used for used defined rationals */
static inline void rat_gmp_canon(mpq_t z){mpq_canonicalize(z);}


/********** standard arithmetic functions for MP rational **********/
/********** + - * / *********/

static inline void rat_gmp_add(const mpq_t z1, const mpq_t z2, mpq_t z){mpq_add(z,z1,z2);}
static inline void rat_gmp_sub(const mpq_t z1, const mpq_t z2, mpq_t z){mpq_sub(z,z1,z2);}
static inline void rat_gmp_mul(const mpq_t z1, const mpq_t z2, mpq_t z){mpq_mul(z,z1,z2);}
static inline void rat_gmp_div(const mpq_t z1, const mpq_t z2, mpq_t z){mpq_div(z,z1,z2);}
void rat_gmp_add_si(const mpq_t z1, int z2, mpq_t z);
void rat_gmp_add_ui(const mpq_t z1, const unsigned int z2, mpq_t z);
void rat_gmp_sub_ui(const mpq_t z1, const unsigned int z2, mpq_t z);
void rat_gmp_mul_si(const mpq_t z1, const int z2, mpq_t z);
void rat_gmp_div_si(const mpq_t z1, const int z2, mpq_t z);
void rat_gmp_si_div(const int z1, const mpq_t z2, mpq_t z);
static inline void rat_gmp_abs(const mpq_t z1, mpq_t z){mpq_abs(z,z1);}
static inline void rat_gmp_neg(const mpq_t z1, mpq_t z){mpq_neg(z,z1);}


/********** more MP rational functions *********/
/********** ^!%>>... **********/

void rat_gmp_power(const mpq_t z1, unsigned int z2, mpq_t z);
void rat_gmp_powerr(const mpq_t z1, const mpq_t z2, mpq_t z);
void rat_gmp_shift(const mpq_t z1, mpq_t z, int p);


/********** output functions for MP rationals ************/

char* rat_gmp_swritee(const mpq_t z, const int w);
char* rat_gmp_sprintf(const mpq_t z);


void rat_gmp_string_2_rat(mpq_t z, const char* s);
void rat_gmp_double_2_rat(mpq_t z, const double d);

static inline void rat_gmp_get_numerator(mpz_t z, const mpq_t z1){mpq_get_num(z,z1);}
static inline void rat_gmp_get_denominator(mpz_t z, const mpq_t z1){mpq_get_den(z,z1);}


/********** copying MP rationals with/without initializing ***********/
static inline void rat_gmp_duplicate_w_init(const mpq_t z1, mpq_ptr *z2){
  *z2=rat_gmp_init(); mpq_set(*z2,z1);}
static inline void rat_gmp_duplicate_wo_init(const mpq_t z1, mpq_t z2){mpq_set(z2,z1);}

/********* sign and comparison of integer and rational */
static inline int rat_gmp_sgn(const mpq_t z){return mpq_sgn(z);}
static inline int rat_gmp_cmp(const mpq_t z1,const mpq_t z2){return mpq_cmp(z1,z2);}

#ifdef __cplusplus
}
#endif

#endif /* ifndef iRRAM_GMP_INT_RAT_H */
