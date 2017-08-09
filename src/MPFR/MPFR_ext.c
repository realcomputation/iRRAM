#include "MPFR_ext.h"


iRRAM_TLS int ext_mpfr_var_count=0;

#define MaxFreeVars 1000
iRRAM_TLS mpfr_ptr mpfr_FreeVars[MaxFreeVars];
iRRAM_TLS int mpfr_FreeVarCount=0L;

iRRAM_TLS int mpfr_TotalAllocVarCount=0;
iRRAM_TLS int mpfr_TotalFreedVarCount=0;

