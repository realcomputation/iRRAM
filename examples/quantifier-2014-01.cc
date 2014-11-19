#include "iRRAM.h"


namespace iRRAM {
  

template <class PARAM> 
class C_SET{
public:
	C_SET();
	C_SET(const PARAM& z);
};

template <> 
class C_SET<REAL>{
public:
	REAL _left, _right;
	C_SET(const REAL& left,const REAL& right){_left=left;_right=right;};
	C_SET(const REAL& z){_left=z;_right=z;};
	C_SET(){};
};


class UNIVERSE {
 public: 
   std::vector<REAL> RV;
   std::vector<INTEGER> IV;
};

template <class T> using TERM = FUNCTION<T,UNIVERSE>;

template <class T> class VARIABLE: public TERM<T>
{};

typedef TERM<LAZY_BOOLEAN> PREDICATE;

template <class PARAM>
class FUNCTIONAL_forall :public FUNCTIONAL_object<LAZY_BOOLEAN,UNIVERSE>
{
public:
	VARIABLE<PARAM> _v;
	C_SET<PARAM> _s;
	PREDICATE _p;

	FUNCTIONAL_forall(
	  const VARIABLE<PARAM>& v,
	  const C_SET<PARAM>& s,  
	  const PREDICATE& p
	){_v=v;_p=p;_s=s;}

	LAZY_BOOLEAN eval(const UNIVERSE& val){return true;}; 
};

template<class PARAM>
inline PREDICATE forall (
		const VARIABLE<PARAM>& v,
		const C_SET<PARAM>& s,
		const PREDICATE& p
		){return new FUNCTIONAL_forall<PARAM>(v,s,p);}

		
		
template <class PARAM>
class FUNCTIONAL_exists :public FUNCTIONAL_object<LAZY_BOOLEAN,UNIVERSE>
{
public:
	VARIABLE<PARAM> _v;
	C_SET<PARAM> _s;
	PREDICATE _p;

	FUNCTIONAL_exists(
	  const VARIABLE<PARAM>& v,
	  const C_SET<PARAM>& s,  
	  const PREDICATE& p
	){_v=v;_p=p;_s=s;}

	LAZY_BOOLEAN eval(const UNIVERSE& val){return true;}; 
};

template<class PARAM>
inline PREDICATE exists (
		const VARIABLE<PARAM>& v,
		const C_SET<PARAM>& s,
		const PREDICATE& p
		){return new FUNCTIONAL_exists<PARAM>(v,s,p);}

		
		
template <class PARAM>
class FUNCTIONAL_assign :public FUNCTIONAL_object<LAZY_BOOLEAN,UNIVERSE>
{
public:
	VARIABLE<PARAM> _v;
	PARAM _s;
	PREDICATE _p;

	FUNCTIONAL_assign(
	  const VARIABLE<PARAM>& v,
	  const PARAM& s,  
	  const PREDICATE& p
	){_v=v;_p=p;_s=s;}

	LAZY_BOOLEAN eval(const UNIVERSE& val){return true;}; 
};

template<class PARAM>
inline PREDICATE assign (
		const VARIABLE<PARAM>& v,
		const PARAM& s,
		const PREDICATE& p
		){return new FUNCTIONAL_assign<PARAM>(v,s,p);}

  
  
  
template <class PARAM,class T>
class FUNCTIONAL_tassign :public FUNCTIONAL_object<T,UNIVERSE>
{
public:
	VARIABLE<PARAM> _v;
	PARAM _s;
	TERM<T> _t;

	FUNCTIONAL_tassign(
	  const VARIABLE<PARAM>& v,
	  const PARAM& s,  
	  const TERM<T>& t
	){_v=v;_t=t;_s=s;}

	T eval(const UNIVERSE& val){return true;}; 
};

template<class PARAM,class T>
inline TERM<T> assign (
		const VARIABLE<PARAM>& v,
		const PARAM& s,
		const TERM<T>& t
		){return new FUNCTIONAL_tassign<PARAM,T>(v,s,t);}
  
  
  
  
  
} //namespace iRRAM

using namespace iRRAM;


// UNIVERSE id_U( const UNIVERSE & x ){ return x; }
// 
// FUNCTION<UNIVERSE,UNIVERSE>  variables=from_algorithm(id_U);
FUNCTION<REAL,REAL>  F_sqrt(iRRAM::sqrt);
void compute(){

//   REAL_EXPRESSION X = projection(variables.RV,0);
//   REAL_EXPRESSION Y = projection(variables,1);
//   REAL_EXPRESSION Z = X*Y+X/Y;
//   REAL_PREDICATE  P =  Z > X  ||  X < REAL(1) ;
// 
//   
//   
//   
// C_SET<REAL> unit_int(REAL(0),REAL(1));
// 
// FUNCTION<LAZY_BOOLEAN,REAL> forall_smaller=forall(X,unit_int,P);
// 
  {
VARIABLE<REAL> X;
PREDICATE P4;
{
VARIABLE<REAL> Y;
VARIABLE<REAL> Z;
REAL A=1;
REAL B=2;
REAL C=3;
TERM<REAL> T = A*X + B*Y;
 VARIABLE<REAL> X; 

PREDICATE P0 = T > C * sqrt(Z);
PREDICATE P1 = forall (X, C_SET<REAL>( 0, 3), P0 ) ;
PREDICATE P2 = exists (Y, C_SET<REAL>( 0, 3), P1 ) ;
 P4 = P2 || X> 1 ;
}


REAL zvalue = pi ( ) ;
PREDICATE P3 = assign ( Z , zvalue , P2 ) ;
REAL delta = 0.001 ;
//cout << P3.value ( delta ) ;
TERM<REAL> T1 = assign (X, zvalue , T ) ;
TERM<REAL> T2 = assign (Y, zvalue , T1 ) ;
//REAL w = T2;
  }
  

}
