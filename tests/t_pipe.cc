
#include <future>
#include <cstdio>

#include <iRRAM/pipe.hh>

using namespace iRRAM;

struct bla {
	int x[16];
};

void compute1(effort_t eff, std::shared_ptr<OSock<REAL>> &e_out)
{
	e_out->put(euler(), eff);
}

void compute2(effort_t eff, const std::shared_ptr<ISock<REAL>> &e_in)
{
	printf("bla\n");
	cout << e_in->get(eff) << "\n";
}

int main(int argc, char **argv)
{
	auto p = std::make_shared<Process>();
	auto q = std::make_shared<Process>();
	auto so = p->out_sock<REAL>();
	auto si = q->connect(so);

	p->exec(argc, argv, compute1, 0, std::ref(so));
	q->exec(argc, argv, compute2, 0, std::cref(si));

//	cout << si->get(0) << "\n";
	si->get(0);
}
