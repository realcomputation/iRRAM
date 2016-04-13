
#include <iRRAM/pipe.h>

void compute1_c(void *cb);
void compute2_c(void *cb);
void compute3_c(void *cb);

int main(int argc, const char *const *argv)
{
	iRRAM_process_t p, q, r;
	iRRAM_osocket_t o, o2, o3;
	iRRAM_isocket_t iq, ir;

	iRRAM_make_process(&p, "p");
	iRRAM_make_process(&q, "q");
	iRRAM_make_process(&r, "r");

	iRRAM_process_out_sock(&o, &p, iRRAM_TYPE_REAL);
	iRRAM_process_out_sock(&o2, &q, iRRAM_TYPE_STRING);
	iRRAM_process_out_sock(&o3, &r, iRRAM_TYPE_STRING);
	iRRAM_process_connect(&iq, &q, &o);
	iRRAM_process_connect(&ir, &r, &o);
	iRRAM_release_isocket(&ir);
	iRRAM_release_isocket(&iq);

	iRRAM_process_exec(&p, argc, argv, compute1_c, &o);
	iRRAM_process_exec(&q, argc, argv, compute2_c, (void *[]){ &iq, &o2 });
	iRRAM_process_exec(&r, argc, argv, compute3_c, (void *[]){ &ir, &o3 });

	sleep(3);

	iRRAM_release_process(&p);
	iRRAM_release_process(&q);
	iRRAM_release_process(&r);

	iRRAM_release_osocket(&o);
	iRRAM_release_osocket(&o2);
	iRRAM_release_osocket(&o3);
}
