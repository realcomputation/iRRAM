/*

GMP_int_ext.c -- extension to the GMP integer functions for the iRRAM library 
 
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

#include <iRRAM/GMP_intrat.h>

#include <strings.h>

#ifndef DEBUG			/* debugging information */
#define DEBUG 0
#endif
#if DEBUG > 0
  int DEBUG_OP = 1;
  unsigned int op_count=0;
  int minshowop = DEBUG ;
#endif

/**************************************************************************/
/* Variables for counting free space                                      */
/**************************************************************************/

iRRAM_TLS mpz_ptr gmp_FreeVarsi[MaxFreeVars];
iRRAM_TLS int gmp_FreeVarCounti=0L;
iRRAM_TLS int int_gmp_var_count=0;

/**************************************************************************/
/* writes integer to a string                                             */
/* writes z to string with length w                                       */
/* Arguments: 1. GMP_integer                                              */
/*            2. int integer                                              */
/* return: char*                                                          */
/**************************************************************************/

char* int_gmp_swritee(const mpz_t z, int w)
{
  char *n, *s;
  int l;
  if (w<1)w=1;
  s=malloc(w+1);
  n=mpz_get_str(NULL, 10, z);
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




/**************************************************************************/
/* shifting GMP_integer by n bits                                         */
/* calculates z = z1 <<(>>) z2                                            */
/* added: 01.02.2001                                                      */
/* Last change: 01.02.2001                                                */
/* Arguments: 1. GMP_integer                                              */
/*            2. GMP_integer                                              */
/*            3. int integer n                                            */
/**************************************************************************/

void int_gmp_shift(const mpz_t z1, mpz_t z, int n)
{
  if (n>=0) mpz_mul_2exp(z, z1, n);
  else mpz_tdiv_q_2exp(z, z1, -n);  /* truncate-div */
}
