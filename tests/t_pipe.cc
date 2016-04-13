
#include <unistd.h>
#include "t_pipe-impl.cc"

int main(int argc, char **argv)
{
	Process_t q = make_process("q");
	Process_t r = make_process("r");
	Process_t p = make_process("p");

	OSock_t<REAL> so = p->out_sock<REAL>();
	p->exec(argc, argv, compute1, std::ref(so));

	OSock_t<std::string> o2 = q->out_sock<std::string>();
	q->exec(argc, argv, compute2, q->connect(so), std::ref(o2));
	q.reset();

	OSock_t<std::string> o3 = r->out_sock<std::string>();
	r->exec(argc, argv, compute3, r->connect(so), std::ref(o3));
	r.reset();

	std::string s2 = dynamic_cast<ISock<std::string>&>(*o2).get(1);
	o2.reset();
	std::string s3 = dynamic_cast<ISock<std::string>&>(*o3).get(1);
	o3.reset();

	std::cout << "s2: " << s2 << "\n"
	          << "s3: " << s3 << "\n"
	          << "p use count: " << p.use_count() << "\n";
	p.reset();
	so.reset();
}
