
#include <iRRAM/pipe.hh>

using namespace iRRAM;

void compute1(OSock_t<REAL> &e_out)
{
	e_out->put(euler());
}

void compute2(const ISock_t<REAL> &e_in)
{
	cout << e_in->get() << "\n";
}

void compute3(const ISock_t<REAL> &e_in)
{
	REAL e = e_in->get();
	cout << power(1/e, 1e4/e) << "\n";
}

extern "C" void compute1_c(void *cb)
{
	iRRAM_osocket_t *o = static_cast<iRRAM_osocket_t *>(cb);
	compute1(*static_cast<OSock_t<REAL> *>(o->s));
}

extern "C" void compute2_c(void *cb)
{
	iRRAM_isocket_t *i = static_cast<iRRAM_isocket_t *>(cb);
	compute2(*static_cast<const ISock_t<REAL> *>(i->s));
}

extern "C" void compute3_c(void *cb)
{
	iRRAM_isocket_t *i = static_cast<iRRAM_isocket_t *>(cb);
	compute2(*static_cast<const ISock_t<REAL> *>(i->s));
}
