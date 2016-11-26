/*

iRRAM_core.h -- basic file for the errorsize arithmetic 
                and for the templates in the iRRAM library
 
Copyright (C) 2001-2009 Norbert Mueller
Copyright     2014-2016 Franz Brausse
 
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
#ifndef IRRAM_CORE_H
#define IRRAM_CORE_H

#include <cstdio>
#include <vector>
#include <cfenv>
//#include <thread>
#include <algorithm>	/* std::min, std::max */
#include <sstream>

#include <iRRAM/lib.h>
#include <iRRAM/version.h>
#include <iRRAM/cache.h>
#include <iRRAM/SWITCHES.h>

namespace iRRAM {

using std::min;
using std::max;

inline int max3(const int a,const int b,const int c)
   { return max(max(a,b),c); }
inline int max4(const int a,const int b,const int c,const int d)
   { return max(max(a,b),max(c,d)); }

extern void resources(double&,unsigned int&);
extern double ln2_time;
extern double pi_time;
void show_statistics();

extern const int iRRAM_prec_steps;
extern const int *const iRRAM_prec_array;

#ifndef NODEBUG
  #define iRRAM_DEBUG0(level,...)                                               \
	do {                                                                    \
		if (iRRAM_unlikely(state.debug>=state.ACTUAL_STACK.inlimit+(level))) {\
			__VA_ARGS__;                                            \
		}                                                               \
	} while (0)
#else
  #define iRRAM_DEBUG0(level,...)
#endif
#define iRRAM_DEBUG1(level,p)	iRRAM_DEBUG0((level),cerr << p)
#define iRRAM_DEBUG2(level,...)	iRRAM_DEBUG0((level),fprintf(stderr,__VA_ARGS__))


struct Iteration {int prec_diff; Iteration (int p){prec_diff=p;}; };

// inline void REITERATE(int p_diff){inReiterate = true; throw Iteration(p_diff); }
#define REITERATE(x)   {state.inReiterate = true;throw Iteration(x);};

}

#include <iRRAM/sizetype.hh>

namespace iRRAM {

/*****************************************/
// iRRAM_exec template

template <class F>
auto iRRAM_exec(F f) -> decltype(f())
{
	state_t &st = state;
	st.thread_data_address = new iRRAM_thread_data_class;

	stiff code(state.prec_start, stiff::abs{});
	fesetround(FE_DOWNWARD);
	// set the correct rounding mode for REAL using double intervals):

	st.cache_active = new cachelist;

	if (iRRAM_unlikely(state.debug > 0)) {
//		std::stringstream s;
//		s << std::this_thread::get_id();
		cerr << "\niRRAM (version " << iRRAM_VERSION_rt
		     << ", backend " << iRRAM_BACKENDS << ")"
//		     << " thread " << s.str()
		     << " starting...\n";
		st.max_prec = st.ACTUAL_STACK.prec_step;
	}

	using RESULT = decltype(f());
	RESULT result;

	st.ACTUAL_STACK.prec_policy = 1;
	st.ACTUAL_STACK.inlimit = 0;
	st.highlevel = (st.ACTUAL_STACK.prec_step > 1);

	while (true) {
		iRRAM::cout.rewind();
		for (int n = 0; n < st.max_active; n++)
			state.cache_active->id[n]->rewind();

		st.inReiterate = false;
		assert(st.ACTUAL_STACK.inlimit == 0);
		assert(st.highlevel == (st.ACTUAL_STACK.prec_step > 1));

		int p_end = 0;
		try {
			result = f();
			if (iRRAM_likely(!st.infinite))
				break;
		} catch (const Iteration &it) {
			p_end = st.ACTUAL_STACK.actual_prec + it.prec_diff;
		} catch (const iRRAM_Numerical_Exception &exc) {
			cerr << "iRRAM exception: " << iRRAM_error_msg[exc.type]
			     << "\n";
			throw;
		}

		assert(st.highlevel == (st.ACTUAL_STACK.prec_step > 1));

		int prec_skip = 0;
		do {
			prec_skip++;
			code.inc_step(4);
		} while ((st.ACTUAL_STACK.actual_prec > p_end) &&
		         (prec_skip != st.prec_skip));

		assert(st.ACTUAL_STACK.inlimit == 0);
		if (iRRAM_unlikely(st.debug > 0)) {
			show_statistics();
			if (st.max_prec <= st.ACTUAL_STACK.prec_step)
				st.max_prec = st.ACTUAL_STACK.prec_step;
			cerr << "increasing precision bound to "
			     << st.ACTUAL_STACK.actual_prec << "["
			     << st.ACTUAL_STACK.prec_step << "]\n";
		}
	}

	iRRAM::cout.reset();
	for (int n = 0; n < st.max_active; n++)
		st.cache_active->id[n]->clear();

	st.max_active = 0;
	delete st.cache_active;
	delete st.thread_data_address;

	if (iRRAM_unlikely(st.debug > 0)) {
		show_statistics();
		cerr << "iRRAM ending \n";
	}

	return result;
}

} /* ! namespace iRRAM */

#endif /* ! iRRAM_CORE_H  */
