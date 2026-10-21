#ifndef _TSD_FORTRAN
#define _TSD_FORTRAN

#ifdef TSD_FORTRAN_IMPL

#define xstr(s)        str(s)
#define str(s)         #s
#define TESTNAME(NAME) TEST_##NAME

#define TSD_TEST_E_INTERFACES                                                  \
  interface;                                                                   \
  subroutine TSD_SetTestSt(NewSt) bind(C, name = "TSD_SetTestSt");             \
  use, intrinsic::ISO_C_BINDING;                                               \
  logical(c_bool), intent(in), value::NewSt;                                   \
  end subroutine TSD_SetTestSt;                                                \
  end            interface

#define TSD_TEST_EF_INTERFACES                                                 \
  interface;                                                                   \
  subroutine TSD_SetTestSt(NewSt) bind(C, name = "TSD_SetTestSt");             \
  use, intrinsic::ISO_C_BINDING;                                               \
  logical(c_bool), intent(in), value::NewSt;                                   \
  end subroutine TSD_SetTestSt;                                                \
  end            interface;                                                    \
  type(C_PTR), intent(in), value::ContextData

#define TEST_E(NAME)                                                           \
  subroutine TESTNAME(NAME)() bind(C, name = xstr(TESTNAME(NAME)));            \
  use, intrinsic::ISO_C_BINDING;                                               \
  use, intrinsic::ISO_FORTRAN_ENV
#define END_TEST_E end subroutine

#define TEST_EF(CONTEXT, NAME)                                                 \
  subroutine TESTNAME(NAME)(ContextData) bind(C, name = xstr(TESTNAME(NAME))); \
  use, intrinsic::ISO_C_BINDING;                                               \
  use, intrinsic::ISO_FORTRAN_ENV
#define END_TEST_EF end subroutine

#define ASSERT_NOEXIT_WRAP(EXPR, FILE, LINE)                                   \
  if (.not.(EXPR))                                                             \
    then;                                                                      \
  call TSD_SetTestSt(logical(.false., kind = c_bool));                         \
  write(ERROR_UNIT, "(A, A, I0, A, A, A)") FILE, ":", LINE, ": Assertion ",    \
      #EXPR, " failed.";                                                       \
  end if
#define ASSERT_EXIT_WRAP(EXPR, FILE, LINE)                                     \
  if (.not.(EXPR))                                                             \
    then;                                                                      \
  call TSD_SetTestSt(logical(.false., kind = c_bool));                         \
  write(ERROR_UNIT, "(A, A, I0, A, A, A)") FILE, ":", LINE, ": Assertion ",    \
      #EXPR, " failed.";                                                       \
  error stop;                                                                  \
  end if

#define ASSERT(EXPR)   ASSERT_NOEXIT_WRAP(EXPR, __FILE__, __LINE__)
#define ASSERT_X(EXPR) ASSERT_EXIT_WRAP(EXPR, __FILE__, __LINE__)

#elif TSD_KERNEL

#include "test/TSD/TSD.h"
#define END_TEST_E
#define END_TEST_EF

#endif

#endif
