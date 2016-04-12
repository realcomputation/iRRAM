
#include "t_pipe-impl.cc"

int main(int argc, char **argv)
{
	Process_t p = make_process();
	Process_t q = make_process();
	Process_t r = make_process();
	OSock_t<REAL> so = p->out_sock<REAL>();

	p->exec(argc, argv, compute1, so);
	q->exec(argc, argv, compute2, q->connect(so));
	r->exec(argc, argv, compute3, r->connect(so));
}
