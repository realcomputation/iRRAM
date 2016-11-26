AC_DEFUN([irram_check_c_threads],[
  AC_CHECK_HEADER([threads.h])
  AS_IF([test $ac_cv_header_threads_h = yes],[
     AC_MSG_CHECKING([whether C threads are usable])
     AC_LINK_IFELSE([AC_LANG_PROGRAM(
[[
#include <threads.h>
int thread_test(void *) {return 0;}
]],
[[
thrd_t thr;
thrd_create(&thr, thread_test, NULL);
thrd_join(thrd_current(), NULL);
]])],[have_C_thrds=yes],[have_C_thrds=no])
     AC_MSG_RESULT([$have_C_thrds])],
    [have_C_thrds=no])])

AC_DEFUN([irram_check_cxx_threads],[
  AC_CHECK_HEADER([thread])
  AS_IF([test $ac_cv_header_thread = yes],[
    AC_MSG_CHECKING([whether C++ threads are usable])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <thread>
int thread_test(void *) {return 0;}
]],[[
std::thread thr(thread_test, nullptr);
std::thread::id tid = std::this_thread::get_id();
thr.join();
]])],[have_CXX_threads=yes],[have_CXX_threads=no])
    AC_MSG_RESULT([$have_CXX_threads])])])




dnl ----------------------------------------------------------------------------
dnl IRRAM_CHECK_SCM(STORAGE_CLASS_MOD,[action-if-ok],[action-if-not-ok],[INPUT])
dnl
dnl Result is stored in a variable named irram_cv_check_scm_C_$1 for C language
dnl and irram_cv_check_scm_CXX_$1 for C++.
dnl
dnl Based on AX_CHECK_COMPILE_FLAG.
dnl ----------------------------------------------------------------------------
AC_DEFUN([IRRAM_CHECK_SCM],[
AC_PREREQ(2.64)dnl for _AC_LANG_PREFIX and AS_VAR_IF
AS_VAR_PUSHDEF([CACHEVAR],[irram_cv_check_scm_[]_AC_LANG_ABBREV[]_$1])
AC_CACHE_CHECK([whether _AC_LANG compiler understands storage class mod $1], CACHEVAR,
  [AC_COMPILE_IFELSE([m4_default([$4],[AC_LANG_PROGRAM([[static ]$1[ int x = 42;]],[[return x;]])])],
    [AS_VAR_SET(CACHEVAR,[yes])],
    [AS_VAR_SET(CACHEVAR,[no])])])
AS_VAR_IF(CACHEVAR,yes,[m4_default([$2], :)],[m4_default([$3], :)])
AS_VAR_POPDEF([CACHEVAR])dnl
])

AC_DEFUN([IRRAM_TLS],[
  AC_LANG_PUSH([C])
  IRRAM_CHECK_SCM([__thread])
  IRRAM_CHECK_SCM([__declspec(thread)])
  IRRAM_CHECK_SCM([_Thread_local])
  AC_LANG_POP([C])

  AC_LANG_PUSH([C++])
  IRRAM_CHECK_SCM([__thread])
  IRRAM_CHECK_SCM([__declspec(thread)])
  IRRAM_CHECK_SCM([thread_local])
  AC_LANG_POP([C++])

  irram_tls=
  AS_IF([test $irram_cv_check_scm_c___thread = yes &&
         test $irram_cv_check_scm_cxx___thread = yes],
    [irram_tls=__thread],
    [AS_IF([test $irram_cv_check_scm_c___declspec_thread_ = yes &&
            test $irram_cv_check_scm_cxx___declspec_thread_ = yes],
      [irram_tls="__declspec(thread)"],
      [AS_IF([test $irram_cv_check_scm_c__Thread_local = yes &&
              test $irram_cv_check_scm_cxx_thread_local = yes],
        [irram_tls=thread_local])])])
])
