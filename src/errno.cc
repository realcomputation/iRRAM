
namespace iRRAM {

#define ERRORDEFINE(x,y) y,
static const char* const error_msg[]={
#include <iRRAM/errno.h>
};

}

extern "C" const char *const *const iRRAM_error_msg = iRRAM::error_msg;
