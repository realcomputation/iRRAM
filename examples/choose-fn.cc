
#include <iRRAM.h>

using namespace iRRAM;

typedef lazy_array<2> lazy_pair;

void compute()
{
	REAL cmp = 1;

	std::function<lazy_pair()> expensive_but_pure_computation = [&]{
		/* does not change global state, just read cmp */
		REAL a = pi();
		REAL b = ln2();
		return lazy_pair { a - cmp < b, a + cmp > b };
	};

	size_t which_case = choose(expensive_but_pure_computation);
	/* which_case will always be the same */

	switch (which_case) {
	case 0: // both false
	case 1: // a + cmp < b
	case 2: // a - cmp > b
		break;
	}
}
