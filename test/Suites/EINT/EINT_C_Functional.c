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

TEST(EINTZeroCount) {
  const double Result = EINT((size_t)0, NULL, 2);
  ASSERT(fabs(Result) < DBL_EPSILON);
}

TEST(EINTFastReturn) {
  const double Sample = (double)2.5;
  const double Result = EINT((size_t)1, (double[]){Sample}, 2);
  ASSERT(fabs(Result - Sample) < DBL_EPSILON);
}

TEST_SUITE(EINTNominalSuite,
           EINTNominalReference);

TEST_SUITE(EINTEdgeCaseSuite,
           EINTZeroCount,
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
  const double Result = EINT_P((size_t)0, NULL, 2);
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
           EINT_PFastReturn);
