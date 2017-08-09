/*

GMP_rat_ext.c -- extension to the GMP rational functions for the 
                 iRRAM library with backend GMP
 
Copyright (C) 2001/2003 Norbert Mueller
 
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

#include <iRRAM/GMP_intrat.h>

#include <strings.h>

#ifndef DEBUG
#define DEBUG 0
#endif
#if DEBUG > 0
  int DEBUG_OP = 1;
  unsigned int op_count=0;
  int minshowop = DEBUG ;
#endif


#define rat_MaxFreeVars 1000
iRRAM_TLS mpq_ptr rat_gmp_FreeVarsi[rat_MaxFreeVars];
iRRAM_TLS int rat_gmp_FreeVarCount=0L;
iRRAM_TLS int rat_gmp_var_count=0;


void rat_gmp_add_si(mpq_srcptr z1, int z2, mpq_ptr z)
{
	mpz_srcptr src_num = mpq_numref(z1);
	mpz_srcptr src_den = mpq_denref(z1);
	mpz_ptr tgt_num = mpq_numref(z);
	mpz_ptr tgt_den = mpq_denref(z);
	mpz_ptr temp = int_gmp_init();

	mpz_set(tgt_den, src_den);
	mpz_mul_si(temp, src_den, z2);
	mpz_add(tgt_num, temp, src_num);

	mpq_canonicalize(z);
	int_gmp_free(temp);
}


/*****************************************************************************/
/* adding rational and int in GMP                                            */
/* added: 18.06.2001                                                         */
/* Last change: 18.06.2001                                                   */
/*****************************************************************************/

void rat_gmp_add_ui(mpq_srcptr z1, const unsigned int z2, mpq_ptr z)
{
	mpz_ptr num;
	mpz_ptr den;
	mpz_ptr temp;
	num=int_gmp_init();
	den=int_gmp_init();
	temp=int_gmp_init();
	mpq_get_num(num,z1);
	mpq_get_den(den,z1);

	mpz_mul_ui(temp,den,z2);
	mpz_add(num,num,temp);

	mpq_set_num(z,num);
	mpq_set_den(z,den); 
	mpq_canonicalize(z);
	int_gmp_free(num);
	int_gmp_free(den);
	int_gmp_free(temp);

return;
}


/*****************************************************************************/
/* subtract rational and int in GMP                                          */
/* added: 18.06.2001                                                         */
/* Last change: 18.06.2001                                                   */
/*****************************************************************************/

void rat_gmp_sub_ui(mpq_srcptr z1, unsigned int z2, mpq_ptr z)
{
	mpz_ptr num;
	mpz_ptr den;
	mpz_ptr temp;
	num=int_gmp_init();
	den=int_gmp_init();
	temp=int_gmp_init();
	mpq_get_num(num,z1);
	mpq_get_den(den,z1);

	mpz_mul_ui(temp,den,z2);
	mpz_sub(num,num,temp);

	mpq_set_num(z,num);
	mpq_set_den(z,den); 
	mpq_canonicalize(z);
	int_gmp_free(num);
	int_gmp_free(den);
	int_gmp_free(temp);

return;
}


/*****************************************************************************/
/* multiplicate rational and int in GMP                                      */
/* added: 18.06.2001                                                         */
/* Last change: 18.06.2001                                                   */
/*****************************************************************************/

void rat_gmp_mul_si(mpq_srcptr z1, int z2, mpq_ptr z)
{
	mpz_ptr num;
	mpz_ptr den;
	num=int_gmp_init();
	den=int_gmp_init();
	mpq_get_num(num,z1);
	mpq_get_den(den,z1);

	mpz_mul_si(num,num,z2);

	mpq_set_num(z,num);
	mpq_set_den(z,den); 
	mpq_canonicalize(z);
	int_gmp_free(num);
	int_gmp_free(den);

return;
}

/*****************************************************************************/
/* dividing rational and int in GMP                                          */
/* added: 18.06.2001                                                         */
/* Last change: 18.06.2001                                                   */
/*****************************************************************************/

void rat_gmp_div_si(mpq_srcptr z1, int z2, mpq_ptr z)
{
	mpz_ptr num;
	mpz_ptr den;
	num=int_gmp_init();
	den=int_gmp_init();
	mpq_get_num(num,z1);
	mpq_get_den(den,z1);

	mpz_mul_si(den,den,z2);

	mpq_set_num(z,num);
	mpq_set_den(z,den); 
	mpq_canonicalize(z);
	int_gmp_free(num);
	int_gmp_free(den);

#if (DEBUG>0)
#endif

return;
}

/*****************************************************************************/
/* dividing rational and int in GMP                                          */
/* added: 18.06.2001                                                         */
/* Last change: 18.06.2001                                                   */
/* ***************************************************************************/

void rat_gmp_si_div(int z1, mpq_srcptr z2, mpq_ptr z)
{
	mpz_ptr num;
	mpz_ptr den;
	num=int_gmp_init();
	den=int_gmp_init();
	mpq_get_num(num,z2);
	mpq_get_den(den,z2);

	mpz_mul_si(den,den,z1);

	mpq_set_num(z,den);	/* invert */
	mpq_set_den(z,num); 
	mpq_canonicalize(z);
	int_gmp_free(num);
	int_gmp_free(den);

return;
}

/*****************************************************************************/
/* power function in GMP                                                     */
/* added: 19.02.2001                                                         */
/* Last change: 19.02.2001                                                   */
/* Comment: base=MP exponent=int                                             */
/*****************************************************************************/

void rat_gmp_power(mpq_srcptr z1, unsigned int z2, mpq_ptr z)
{
	int sign;
	mpz_ptr num;
	mpz_ptr den;
	mpz_ptr temp;
	num=int_gmp_init();
	den=int_gmp_init();
	temp=int_gmp_init();
	mpq_get_num(num,z1);
	mpq_get_den(den,z1);
	if (mpz_sgn(num)<0) sign=-1; else sign=1;
	mpz_set_si(temp,sign);
	if (sign==-1)mpz_mul(num,num,temp); /***** Nachprfen*/
	mpz_pow_ui(num,num,z2);
	mpz_pow_ui(den,den,z2);
	if (sign==-1)mpz_mul(num,num,temp);
	mpq_set_num(z,num);
	mpq_set_den(z,den);
	mpq_canonicalize(z);
	int_gmp_free(num);
	int_gmp_free(den);
}



char* rat_gmp_swritee(mpq_srcptr z, int w)
{
  char *n, *s;
  int l;
  if (w<1)w=1;
  s=malloc(w+1);
  n=mpq_get_str(NULL, 10, z);
  l=strlen(n);
  if (l>w){
    strncpy(s,n,w);
    s[w-1]='*';
    s[w]='\0';
  }else{
    memset(s,' ',w-l);
    strncpy(&s[w-l],n,l);
    s[w]='\0';
  } 
  free(n); 
  return s;
}


char* rat_gmp_sprintf(mpq_srcptr z)
{
	return mpq_get_str(NULL, 10, z);
}


void rat_gmp_shift(mpq_srcptr z1, mpq_ptr z, int n)
{
	mpz_ptr num,den;
	num=int_gmp_init();
	den=int_gmp_init();
	mpq_get_num(num,z1);
	mpq_get_den(den,z1);

  	if (n>=0) mpz_mul_2exp(num, num, n);
	else mpz_mul_2exp(den, den, -n);	

	mpq_set_num(z,num);
	mpq_set_den(z,den);
	mpq_canonicalize(z);

	int_gmp_free(num);
	int_gmp_free(den);
}

void rat_gmp_string_2_rat(mpq_ptr z, const char* s)
{
	mpq_set_str(z,s,10);
	mpq_canonicalize(z);
}

