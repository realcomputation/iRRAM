/*

iRRAM_lib.h -- central header file for the iRRAM library
 
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


/*
Authors:  all by Norbert, except: 

  2003-07 INTERVAL extensions partly by Shao Qi

  2001-07 INTEGER and RATIONAL partly by Tom van Diessen


*/


/*!
 * \defgroup types Data Types
 * \brief Discrete and continuous data types.
 */

/*!
 * \defgroup maths Mathematical functions
 */

/*!
 * \defgroup debug Debug
 */



#ifndef iRRAM_LIB_H
#define iRRAM_LIB_H

#include <string>
#include <cstdint>
#include <cfenv>
#include <vector>

#include <iRRAM/LAZYBOOLEAN.h>
#include <iRRAM/REAL.h>
#include <iRRAM/DYADIC.h>
#include <iRRAM/INTEGER.h>
#include <iRRAM/RATIONAL.h>
#include <iRRAM/REALMATRIX.h>
#include <iRRAM/SPARSEREALMATRIX.h>
#include <iRRAM/COMPLEX.h>
#include <iRRAM/INTERVAL.h>
#include <iRRAM/STREAMS.h>
#include <iRRAM/SWITCHES.h>
#include <iRRAM/FUNCTION.h>
#include <iRRAM/helper-templates.hh>
#include <iRRAM/limit_templates.h>

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

} // namespace iRRAM

#endif /* ! iRRAM_LIB_H */
