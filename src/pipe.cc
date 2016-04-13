
#include <iRRAM/pipe.hh>

namespace iRRAM {

struct Process::hide_constructor {};

Process::Process(hide_constructor) {}

Process::~Process()
{
	cancelled = true;
	output_requested.notify_one();
	if (thr.joinable())
		thr.join();
	printf("~Process()\n");
}

void Process::run(effort_t e)
{
	{
		std::unique_lock<decltype(mtx_outputs)> lock(mtx_outputs);
		if (e > max_effort_requested) {
			max_effort_requested = e;
			output_requested.notify_one();
		}
	}
	printf("Process::run(%u)\n", e);
}

Process_t make_process()
{
	return std::make_shared<Process>(Process::hide_constructor{});
}

template class Process::Sock<int>;
template class Process::Sock<double>;
template class Process::Sock<INTEGER>;
template class Process::Sock<RATIONAL>;
template class Process::Sock<DYADIC>;
template class Process::Sock<LAZY_BOOLEAN>;
template class Process::Sock<REAL>;
template class Process::Sock<COMPLEX>;
template class Process::Sock<REALMATRIX>;
template class Process::Sock<SPARSEREALMATRIX>;
template class Process::Sock<TM>;

template <typename T>
static inline void * osock(Process_t &p) { return new OSock_t<T>(std::move(p->out_sock<T>())); }

template <typename T>
static inline void * isock(Process_t &p, void *s)
{
	return new ISock_t<T>(std::move(p->connect(*static_cast<OSock_t<T> *>(s))));
}

}

extern "C" {

using namespace iRRAM;

int iRRAM_make_process(iRRAM_process_t *p)
{
	p->p = new Process_t(make_process());
	return 0;
}

int iRRAM_release_process(iRRAM_process_t *p)
{
	delete static_cast<Process_t *>(p->p);
	return 0;
}

int iRRAM_process_out_sock(iRRAM_osocket_t *s, const iRRAM_process_t *_p, enum iRRAM_type type)
{
	Process_t &p = *static_cast<Process_t *>(_p->p);
	switch (type) {
	case iRRAM_TYPE_INT             : s->s = osock<int>(p); break;
	case iRRAM_TYPE_DOUBLE          : s->s = osock<double>(p); break;
	case iRRAM_TYPE_INTEGER         : s->s = osock<INTEGER>(p); break;
	case iRRAM_TYPE_RATIONAL        : s->s = osock<RATIONAL>(p); break;
	case iRRAM_TYPE_DYADIC          : s->s = osock<DYADIC>(p); break;
	case iRRAM_TYPE_LAZY_BOOLEAN    : s->s = osock<LAZY_BOOLEAN>(p); break;
	case iRRAM_TYPE_REAL            : s->s = osock<REAL>(p); break;
	case iRRAM_TYPE_COMPLEX         : s->s = osock<COMPLEX>(p); break;
	case iRRAM_TYPE_REALMATRIX      : s->s = osock<REALMATRIX>(p); break;
	case iRRAM_TYPE_SPARSEREALMATRIX: s->s = osock<SPARSEREALMATRIX>(p); break;
	case iRRAM_TYPE_TM              : s->s = osock<TM>(p); break;
	default:
		return -EINVAL;
	}
	s->t = type;
	return 0;
}

int iRRAM_process_connect(iRRAM_isocket_t *s, const iRRAM_process_t *_p, const iRRAM_osocket_t *o)
{
	Process_t &p = *static_cast<Process_t *>(_p->p);
	switch (o->t) {
	case iRRAM_TYPE_INT             : s->s = isock<int>(p, o->s); break;
	case iRRAM_TYPE_DOUBLE          : s->s = isock<double>(p, o->s); break;
	case iRRAM_TYPE_INTEGER         : s->s = isock<INTEGER>(p, o->s); break;
	case iRRAM_TYPE_RATIONAL        : s->s = isock<RATIONAL>(p, o->s); break;
	case iRRAM_TYPE_DYADIC          : s->s = isock<DYADIC>(p, o->s); break;
	case iRRAM_TYPE_LAZY_BOOLEAN    : s->s = isock<LAZY_BOOLEAN>(p, o->s); break;
	case iRRAM_TYPE_REAL            : s->s = isock<REAL>(p, o->s); break;
	case iRRAM_TYPE_COMPLEX         : s->s = isock<COMPLEX>(p, o->s); break;
	case iRRAM_TYPE_REALMATRIX      : s->s = isock<REALMATRIX>(p, o->s); break;
	case iRRAM_TYPE_SPARSEREALMATRIX: s->s = isock<SPARSEREALMATRIX>(p, o->s); break;
	case iRRAM_TYPE_TM              : s->s = isock<TM>(p, o->s); break;
	default:
		return -EINVAL;
	}
	s->t = o->t;
	return 0;
}

int iRRAM_release_osocket(iRRAM_osocket_t *s)
{
	switch (s->t) {
	case iRRAM_TYPE_INT             : delete static_cast<OSock_t<int             > *>(s->s); break;
	case iRRAM_TYPE_DOUBLE          : delete static_cast<OSock_t<double          > *>(s->s); break;
	case iRRAM_TYPE_INTEGER         : delete static_cast<OSock_t<INTEGER         > *>(s->s); break;
	case iRRAM_TYPE_RATIONAL        : delete static_cast<OSock_t<RATIONAL        > *>(s->s); break;
	case iRRAM_TYPE_DYADIC          : delete static_cast<OSock_t<DYADIC          > *>(s->s); break;
	case iRRAM_TYPE_LAZY_BOOLEAN    : delete static_cast<OSock_t<LAZY_BOOLEAN    > *>(s->s); break;
	case iRRAM_TYPE_REAL            : delete static_cast<OSock_t<REAL            > *>(s->s); break;
	case iRRAM_TYPE_COMPLEX         : delete static_cast<OSock_t<COMPLEX         > *>(s->s); break;
	case iRRAM_TYPE_REALMATRIX      : delete static_cast<OSock_t<REALMATRIX      > *>(s->s); break;
	case iRRAM_TYPE_SPARSEREALMATRIX: delete static_cast<OSock_t<SPARSEREALMATRIX> *>(s->s); break;
	case iRRAM_TYPE_TM              : delete static_cast<OSock_t<TM              > *>(s->s); break;
	default:
		return -EINVAL;
	}
	return 0;
}

int iRRAM_release_isocket(iRRAM_isocket_t *s)
{
	switch (s->t) {
	case iRRAM_TYPE_INT             : delete static_cast<ISock_t<int             > *>(s->s); break;
	case iRRAM_TYPE_DOUBLE          : delete static_cast<ISock_t<double          > *>(s->s); break;
	case iRRAM_TYPE_INTEGER         : delete static_cast<ISock_t<INTEGER         > *>(s->s); break;
	case iRRAM_TYPE_RATIONAL        : delete static_cast<ISock_t<RATIONAL        > *>(s->s); break;
	case iRRAM_TYPE_DYADIC          : delete static_cast<ISock_t<DYADIC          > *>(s->s); break;
	case iRRAM_TYPE_LAZY_BOOLEAN    : delete static_cast<ISock_t<LAZY_BOOLEAN    > *>(s->s); break;
	case iRRAM_TYPE_REAL            : delete static_cast<ISock_t<REAL            > *>(s->s); break;
	case iRRAM_TYPE_COMPLEX         : delete static_cast<ISock_t<COMPLEX         > *>(s->s); break;
	case iRRAM_TYPE_REALMATRIX      : delete static_cast<ISock_t<REALMATRIX      > *>(s->s); break;
	case iRRAM_TYPE_SPARSEREALMATRIX: delete static_cast<ISock_t<SPARSEREALMATRIX> *>(s->s); break;
	case iRRAM_TYPE_TM              : delete static_cast<ISock_t<TM              > *>(s->s); break;
	default:
		return -EINVAL;
	}
	return 0;
}

int iRRAM_process_exec(const iRRAM_process_t *_p,
                       int argc, const char *const *argv,
                       void (*compute)(void *cb_data), void *cb_data)
{
	Process_t &p = *static_cast<Process_t *>(_p->p);
	p->exec(argc, argv, *compute, cb_data);
	return 0;
}

int iRRAM_il_interpret(const char *code, const struct iRRAM_il_env_entry *ios);


}
