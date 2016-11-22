
#include <iRRAM.h>

using namespace iRRAM;

#define ERROR(...) do { printf(__VA_ARGS__); exit(1); } while (0)

static bool run_sqrt(COMPLEX z)
{
	return imag(sqrt(z)) < 0;
}

static bool test_sqrt()
{
	int p = iRRAM_prec_array[5]-11;
	COMPLEX z(-1, REAL(1) << p); /* switches from choice 4 to 1 */
#if 0
	static int run = 0;
	static int lt[5];
	lt[run] = run_sqrt(z);
	printf("run %d, lt: %d\n", run, lt[run]);
	if (++run < sizeof(lt)/sizeof(*lt)) {
		REITERATE(0);
	} else
		return lt[0] == lt[1];
#else
	bool lt1 = run_sqrt(z);
	printf("got lt1: %d\n", lt1);

	/* force increase of precision */
	bool inc_prec = REAL(1) - (REAL(1) - (REAL(1) >> 1000)) > 0;
	assert(inc_prec);

	bool lt4 = run_sqrt(z);
	printf("got lt4: %d\n", lt4);

	return lt1 == lt4;
#endif
}

void compute()
{
	if (!test_sqrt()) ERROR("test_sqrt()");

	cout << "t_COMPLEX: passed\n";
}
