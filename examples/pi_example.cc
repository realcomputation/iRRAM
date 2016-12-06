
#include <iRRAM.h>

using namespace iRRAM;

REAL maxapprox(int prec, const REAL & x, const REAL & y)
{
	if (positive(x - y, prec))
		return x;
	else
		return y;
}

REAL max(const REAL & x, const REAL & y) { return limit(maxapprox, x, y); }

void compute()
{
	cout << "Small test program showing some features...\n";

	REAL x1(3.14159);
	REAL x2("3.14159");
	REAL x3 = pi();

	cout << setRwidth(30) << "\n\n";
	cout << x1 << "\n";
	cout << x2 << "\n";
	cout << max(x3, x2) << "\n\n";
}
