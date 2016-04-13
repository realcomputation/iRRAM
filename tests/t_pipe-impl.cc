
#include <iRRAM/pipe.hh>

using namespace iRRAM;

void compute1(OSock_t<REAL> &e_out)
{
	e_out->put(euler());
}

void compute2(const ISock_t<REAL> &e_in, OSock_t<std::string> &o2)
{
	// cout << e_in->get() << "\n";
	o2->put(swrite(e_in->get(), 20));
}

void compute3(const ISock_t<REAL> &e_in, OSock_t<std::string> &o3)
{
	REAL e = e_in->get();
	// cout << power(1/e, 1e4/e) << "\n";
	o3->put(swrite(power(1/e, 1e4/e), 20));
}

extern "C" void compute1_c(void *cb)
{
	iRRAM_osocket_t *o = static_cast<iRRAM_osocket_t *>(cb);
	compute1(*static_cast<OSock_t<REAL> *>(o->s));
}

extern "C" void compute2_c(void *cb)
{
	void **d = static_cast<void **>(cb);
	iRRAM_isocket_t *i = static_cast<iRRAM_isocket_t *>(d[0]);
	iRRAM_osocket_t *o = static_cast<iRRAM_osocket_t *>(d[1]);
	compute2(
		*static_cast<const ISock_t<REAL> *>(i->s),
		*static_cast<OSock_t<std::string> *>(o->s));
}

extern "C" void compute3_c(void *cb)
{
	void **d = static_cast<void **>(cb);
	iRRAM_isocket_t *i = static_cast<iRRAM_isocket_t *>(d[0]);
	iRRAM_osocket_t *o = static_cast<iRRAM_osocket_t *>(d[1]);
	compute3(
		*static_cast<const ISock_t<REAL> *>(i->s),
		*static_cast<OSock_t<std::string> *>(o->s));
}
