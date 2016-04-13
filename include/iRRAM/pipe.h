
#ifndef iRRAM_PIPE_H
#define iRRAM_PIPE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned iRRAM_effort_t;

enum iRRAM_type {
	iRRAM_TYPE_INT,
	iRRAM_TYPE_DOUBLE,
	iRRAM_TYPE_STRING,
	iRRAM_TYPE_INTEGER,
	iRRAM_TYPE_RATIONAL,
	iRRAM_TYPE_DYADIC,
	iRRAM_TYPE_LAZY_BOOLEAN,
	iRRAM_TYPE_REAL,
	iRRAM_TYPE_COMPLEX,
	iRRAM_TYPE_REALMATRIX,
	iRRAM_TYPE_SPARSEREALMATRIX,
	iRRAM_TYPE_TM,
};

typedef struct { void *p;                    } iRRAM_process_t;
typedef struct { void *s; enum iRRAM_type t; } iRRAM_osocket_t;
typedef struct { void *s; enum iRRAM_type t; } iRRAM_isocket_t;

typedef union {
	int i;
	double d;
	char *s;
} iRRAM_simple_t;

int iRRAM_make_process(iRRAM_process_t *, const char *id);

int iRRAM_process_out_sock(iRRAM_osocket_t *,
                           const iRRAM_process_t *process,
                           enum iRRAM_type type);

int iRRAM_process_connect(iRRAM_isocket_t *,
                          const iRRAM_process_t *process,
                          const iRRAM_osocket_t *osock);

int iRRAM_process_exec(const iRRAM_process_t *process,
                       int argc, const char *const *argv,
                       void (*compute)(void *cb_data), void *cb_data);

int iRRAM_osock_get(iRRAM_simple_t *result, iRRAM_osocket_t *os, iRRAM_effort_t effort);

int iRRAM_release_process(iRRAM_process_t *process);
int iRRAM_release_osocket(iRRAM_osocket_t *sock);
int iRRAM_release_isocket(iRRAM_isocket_t *sock);

struct iRRAM_il_env_entry {
	const char *id;
	union {
		iRRAM_osocket_t osock;
		iRRAM_isocket_t isock;
	};
};

int iRRAM_il_interpret(const char *code, const struct iRRAM_il_env_entry *ios);


#ifdef __cplusplus
}
#endif

#endif
