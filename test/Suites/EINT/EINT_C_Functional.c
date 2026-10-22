#include "src/EINT/ExtrapolationIntegration.h"
#include "test/TSD/TSD.h"
#include <float.h>
#include <math.h>

TEST(EINTNominalReference) {
  const size_t Count = 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 + 1;
  double       Array[Count];

  const double UpperBound = (double)10.0,
               Reference  = (double)1.658347594218874049;

  const double Tolerance = DBL_EPSILON * (double)10000;

  double x, Result;
  size_t i;

  Array[0] = (double)1.0;
  for (i = 2; i <= Count; i++) {
    x            = UpperBound * (double)(i - 1) / (double)(Count - 1);
    Array[i - 1] = sin(x) / x;
  }

  Result = UpperBound * EINT(Count, Array, 2);

  ASSERT((double)100.0 * fabs(Result - Reference) / Reference < Tolerance);
}

TEST(EINTPrimeBaseReference) {
  const size_t Count = 3 * 3 * 3 + 1;
  double       Array[Count];

  /*27 = 3^3: base 3 unlocks three extrapolation steps, which remove the
    h^2, h^4 and h^6 terms of the trapezoidal error, so a degree-6
    polynomial integrates exactly. The same grid read with the default base
    (2, and 27 is odd, so n_p = 0) is the bare trapezoidal rule and lands
    4.8e-3 off: a silent fallback to base 2 cannot pass.*/
  const double Reference = (double)1.0 / (double)7.0;

  const double Tolerance = DBL_EPSILON * (double)1000;

  double x;
  size_t i;

  for (i = 1; i <= Count; i++) {
    x            = (double)(i - 1) / (double)(Count - 1);
    Array[i - 1] = x * x * x * x * x * x;
  }

  ASSERT(fabs(EINT(Count, Array, 3) - Reference) < Tolerance);
}

TEST(EINTZeroCount) {
  double       Array[1];
  const double Result = EINT((size_t)0, Array, 2);
  ASSERT(fabs(Result) < DBL_EPSILON);
}

TEST(EINTNULLArray) {
  const double Result = EINT((size_t)1, NULL, 2);
  ASSERT(fabs(Result) < DBL_EPSILON);
}

TEST(EINTFastReturn) {
  const double Sample = (double)2.5;
  const double Result = EINT((size_t)1, (double[]){Sample}, 2);
  ASSERT(fabs(Result - Sample) < DBL_EPSILON);
}

TEST_SUITE(EINTNominalSuite,
           EINTNominalReference,
           EINTPrimeBaseReference);

TEST_SUITE(EINTEdgeCaseSuite,
           EINTZeroCount,
           EINTNULLArray,
           EINTFastReturn);

TEST(EINT_PNominalReference) {
  const size_t Count = 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 + 1;
  double       Array[Count];

  const double UpperBound = (double)10.0,
               Reference  = (double)1.658347594218874049;

  const double Tolerance = DBL_EPSILON * (double)10000;

  double x, Result;
  size_t i;

  Array[0] = (double)1.0;
  for (i = 2; i <= Count; i++) {
    x            = UpperBound * (double)(i - 1) / (double)(Count - 1);
    Array[i - 1] = sin(x) / x;
  }

  Result = UpperBound * EINT_P(Count, Array, 2);

  ASSERT((double)100.0 * fabs(Result - Reference) / Reference < Tolerance);
}

TEST(EINT_PZeroCount) {
  double       Array[1];
  const double Result = EINT_P((size_t)0, Array, 2);
  ASSERT(fabs(Result) < DBL_EPSILON);
}

TEST(EINT_PNULLArray) {
  const double Result = EINT_P((size_t)1, NULL, 2);
  ASSERT(fabs(Result) < DBL_EPSILON);
}

TEST(EINT_PFastReturn) {
  const double Sample = (double)2.5;
  const double Result = EINT_P((size_t)1, (double[]){Sample}, 2);
  ASSERT(fabs(Result - Sample) < DBL_EPSILON);
}

TEST_SUITE(EINT_PNominalSuite,
           EINT_PNominalReference);

TEST_SUITE(EINT_PEdgeCaseSuite,
           EINT_PZeroCount,
           EINT_PNULLArray,
           EINT_PFastReturn);
