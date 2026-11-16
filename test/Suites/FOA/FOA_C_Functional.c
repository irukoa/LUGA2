#include "src/FOA/FunctionalOptimizationAlgorithms.h"
#include "test/TSD/TSD.h"
#include <float.h>
#include <math.h>

// Number of evaluations that RecordingObjective can memorize.
#define FOA_RecordLimit ((size_t)64)

// Points passed to RecordingObjective, in order, and how many were recorded.
static double RecordedPoints[FOA_RecordLimit];
static size_t RecordedCount;

// Context observed by NullContextObjective.
static bool ObservedNullContext;

static void ResetRecording(void) {
  size_t i;

  RecordedCount = 0;
  for (i = 0; i < FOA_RecordLimit; i++) {
    RecordedPoints[i] = 0.0;
  }
}

// Records its argument before evaluating x - 2, so that the sequence of the
// evaluations and not only their number can be checked.
static double RecordingObjective(const size_t Dimension,
                                 const double EvaluationPoint[Dimension],
                                 void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  if (RecordedCount < FOA_RecordLimit) {
    RecordedPoints[RecordedCount] = EvaluationPoint[0];
  }
  RecordedCount++;

  return EvaluationPoint[0] - (double)2.0;
}

// x^2 - 1, whose zeros are -1 and 1.
static double QuadraticObjective(const size_t Dimension,
                                 const double EvaluationPoint[Dimension],
                                 void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] * EvaluationPoint[0] - (double)1.0;
}

// x - 2.
static double LineObjective(const size_t Dimension,
                            const double EvaluationPoint[Dimension],
                            void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] - (double)2.0;
}

// x0 + x1 - 5: the zero depends on both components, so a search which treats
// the point as a single scalar cannot find it.
static double SumObjective(const size_t Dimension,
                           const double EvaluationPoint[Dimension],
                           void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] + EvaluationPoint[1] - (double)5.0;
}

// x^2 - C, with C read from the context.
static double OffsetObjective(const size_t Dimension,
                              const double EvaluationPoint[Dimension],
                              void        *UserContext) {
  const double Offset = *(const double *)UserContext;

  (void)Dimension;

  return EvaluationPoint[0] * EvaluationPoint[0] - Offset;
}

// -1 below 2, +1 at and above 2: a sign change without a zero.
static double StepObjective(const size_t Dimension,
                            const double EvaluationPoint[Dimension],
                            void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] < (double)2.0 ? (double)-1.0 : (double)1.0;
}

// Same sign change as StepObjective, with values whose product overflows.
static double HugeStepObjective(const size_t Dimension,
                                const double EvaluationPoint[Dimension],
                                void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] < (double)2.0 ? (double)-1.0E200 : (double)1.0E200;
}

// Same sign change as StepObjective, with values whose product underflows.
static double TinyStepObjective(const size_t Dimension,
                                const double EvaluationPoint[Dimension],
                                void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] < (double)2.0 ? (double)-1.0E-200
                                          : (double)1.0E-200;
}

// Undefined above 2: the sign change is lost.
static double NaNObjective(const size_t Dimension,
                           const double EvaluationPoint[Dimension],
                           void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] < (double)2.0 ? (double)-1.0 : (double)NAN;
}

// No sign change at all.
static double ConstantObjective(const size_t Dimension,
                                const double EvaluationPoint[Dimension],
                                void        *UserContext) {
  (void)Dimension;
  (void)EvaluationPoint;
  (void)UserContext;

  return (double)1.0;
}

// Notices whether the context reached it.
static double NullContextObjective(const size_t Dimension,
                                   const double EvaluationPoint[Dimension],
                                   void        *UserContext) {
  (void)Dimension;

  ObservedNullContext = (UserContext == NULL);

  return EvaluationPoint[0] - (double)2.0;
}
// -1 below 1e16 + 2, +1 at and above: the bracket [1e16, 1e16 + 2] holds a
// sign change but no representable point between its ends.
static double UlpStepObjective(const size_t Dimension,
                               const double EvaluationPoint[Dimension],
                               void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  return EvaluationPoint[0] < (double)1.0E16 + (double)2.0 ? (double)-1.0
                                                           : (double)1.0;
}

// -1 below 2, NaN between 2 and 3, +1 above 3: the sign change is there, but
// an interval without a value stands in its way.
static double GapNaNObjective(const size_t Dimension,
                              const double EvaluationPoint[Dimension],
                              void        *UserContext) {
  (void)Dimension;
  (void)UserContext;

  if (EvaluationPoint[0] < (double)2.0) {
    return (double)-1.0;
  }
  if (EvaluationPoint[0] > (double)3.0) {
    return (double)1.0;
  }

  return (double)NAN;
}

// A single component, a moderate tolerance and enough iterations to reach it:
// the residual stop is the binding rule.
TEST(FOAOneDimensionalNominal) {
  double       A[1] = {(double)0.0}, B[1] = {(double)2.5}, Root[1];
  const size_t MaxIterations = 200;
  const double Tolerance     = (double)1.0E-9;

  BisectionRootFinding(1, A, B, QuadraticObjective, MaxIterations, Tolerance,
                       Root, NULL);

  ASSERT(!isnan(Root[0]));
  ASSERT(fabs(Root[0] - (double)1.0) <= (double)1.0E-9);
  ASSERT(fabs(QuadraticObjective(1, Root, NULL)) <= (double)1.0E-9);
}

// Two components: the search runs along the segment which joins the pivots,
// so the zero of a function of both components is reachable.
TEST(FOAMultidimensionalSegmentNominal) {
  double       A[2] = {(double)0.0, (double)0.0};
  double       B[2] = {(double)4.0, (double)6.0};
  double       Root[2];
  const size_t MaxIterations = 200;
  const double Tolerance     = (double)0.0;

  BisectionRootFinding(2, A, B, SumObjective, MaxIterations, Tolerance, Root,
                       NULL);

  // The first midpoint is (2, 3), where the objective vanishes exactly.
  ASSERT(Root[0] == (double)2.0);
  ASSERT(Root[1] == (double)3.0);
}

// The pivots may be supplied in either order: the segment is the same, and so
// is the answer.
TEST(FOAReversedPivotNominal) {
  double       A[1] = {(double)5.0}, B[1] = {(double)-1.0}, Fwd[1], Bwd[1];
  const size_t MaxIterations = 200;
  const double Tolerance     = (double)1.0E-9;

  BisectionRootFinding(1, A, B, LineObjective, MaxIterations, Tolerance, Fwd,
                       NULL);
  BisectionRootFinding(1, B, A, LineObjective, MaxIterations, Tolerance, Bwd,
                       NULL);

  ASSERT(fabs(Fwd[0] - (double)2.0) <= (double)1.0E-9);
  ASSERT(fabs(Bwd[0] - (double)2.0) <= (double)1.0E-9);
}

// The context reaches the callback: the objective reads its constant from it,
// and a NULL context would make the callback dereference NULL.
TEST(FOAContextForwardingNominal) {
  double       A[1] = {(double)0.0}, B[1] = {(double)5.0}, Root[1];
  const size_t MaxIterations = 200;
  const double Tolerance     = (double)1.0E-12;
  double       Constant      = (double)9.0;

  BisectionRootFinding(1, A, B, OffsetObjective, MaxIterations, Tolerance, Root,
                       &Constant);

  // The zero of x^2 - 9 on [0, 5] is 3, not the zero of x^2 - 1.
  ASSERT(fabs(Root[0] - (double)3.0) <= (double)1.0E-9);
}

// The context is NULL when the caller does not supply one.
TEST(FOANullContextNominal) {
  double       A[1] = {(double)0.0}, B[1] = {(double)4.0}, Root[1];
  const size_t MaxIterations = 200;
  const double Tolerance     = (double)0.0;

  ObservedNullContext = false;

  BisectionRootFinding(1, A, B, NullContextObjective, MaxIterations, Tolerance,
                       Root, NULL);

  ASSERT(ObservedNullContext);
  ASSERT(Root[0] == (double)2.0);
}

// The two pivots are evaluated first, then one point per iteration, and the
// last estimate is the point of the last iteration. The values below are
// exact in binary, so the sequence is pinned without a tolerance.
TEST(FOACallbackProtocolNominal) {
  double       A[1] = {(double)0.0}, B[1] = {(double)3.0}, Root[1];
  const size_t MaxIterations = 3;
  const double Tolerance     = (double)0.0;

  ResetRecording();

  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations, Tolerance,
                       Root, NULL);

  ASSERT(RecordedCount == 5);
  ASSERT(RecordedPoints[0] == (double)0.0);
  ASSERT(RecordedPoints[1] == (double)3.0);
  ASSERT(RecordedPoints[2] == (double)1.5);
  ASSERT(RecordedPoints[3] == (double)2.25);
  ASSERT(RecordedPoints[4] == (double)1.875);
  ASSERT(Root[0] == (double)1.875);
}

TEST_SUITE(FOACFunctionalNominalSuite,
           FOAOneDimensionalNominal,
           FOAMultidimensionalSegmentNominal,
           FOAReversedPivotNominal,
           FOAContextForwardingNominal,
           FOANullContextNominal,
           FOACallbackProtocolNominal);

// A zero at a pivot is an exact answer: the sign change is at the pivot
// itself, the pivot is returned unchanged, and no iteration is needed.
TEST(FOAExactZeroAtPivotEdgeCase) {
  double       A[1] = {(double)2.0}, B[1] = {(double)5.0}, Root[1];
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  ResetRecording();
  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations, Tolerance,
                       Root, NULL);

  ASSERT(Root[0] == (double)2.0);
  ASSERT(RecordedCount == 2);

  // The same at the right pivot, where x - 2 already vanishes.
  A[0] = (double)-1.0;
  B[0] = (double)2.0;

  ResetRecording();
  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations, Tolerance,
                       Root, NULL);

  ASSERT(Root[0] == (double)2.0);
  ASSERT(RecordedCount == 2);
}

// The second midpoint of the bracket lands exactly on the zero: the residual
// stop fires with Tolerance = 0, although no pivot vanishes.
TEST(FOAExactZeroAtMidpointEdgeCase) {
  double       A[1] = {(double)3.0}, B[1] = {(double)-1.0}, Root[1];
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  BisectionRootFinding(1, A, B, LineObjective, MaxIterations, Tolerance, Root,
                       NULL);

  ASSERT(Root[0] == (double)2.0);
}

// Two adjacent representable pivots: no point lies strictly between them, so
// the search returns the last estimate instead of halving the bracket forever.
TEST(FOACollapsedBracketEdgeCase) {
  double       A[1] = {(double)1.0E16}, B[1] = {(double)1.0E16 + (double)2.0};
  double       Root[1];
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  BisectionRootFinding(1, A, B, UlpStepObjective, MaxIterations, Tolerance,
                       Root, NULL);

  // The midpoint of the two ends rounds back to the left one: the bracket
  // cannot be split, and the result is one of its ends.
  ASSERT(Root[0] == A[0]);
}

// The cap stops the search before the tolerance is met: the estimate is the
// fourth midpoint of the segment, and it is within |B - A| / 2^4 of the zero.
// The values are exact in binary, so they are pinned without a tolerance.
TEST(FOAIterationCapAccuracyEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)7.0}, Root[1];
  const size_t MaxIterations = 4;
  const double Tolerance     = (double)0.0;

  BisectionRootFinding(1, A, B, LineObjective, MaxIterations, Tolerance, Root,
                       NULL);

  ASSERT(Root[0] == (double)2.1875);
  ASSERT(fabs(Root[0] - (double)2.0) <= (double)7.0 / (double)16.0);
}

// An objective which only reports a sign has a sign change but no zero: the
// residual stop never fires, and the bracket collapses onto the change.
TEST(FOASignOnlyObjectiveEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)4.0}, Root[1];
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  BisectionRootFinding(1, A, B, StepObjective, MaxIterations, Tolerance, Root,
                       NULL);

  ASSERT(fabs(Root[0] - (double)2.0) <= DBL_EPSILON);
}

// Values whose product overflows or underflows still bracket the sign change:
// the ends are compared through their sign, not through their product.
TEST(FOAScaledSignObjectiveEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)4.0};
  double       Huge[1], Tiny[1];
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  BisectionRootFinding(1, A, B, HugeStepObjective, MaxIterations, Tolerance,
                       Huge, NULL);
  BisectionRootFinding(1, A, B, TinyStepObjective, MaxIterations, Tolerance,
                       Tiny, NULL);

  ASSERT(fabs(Huge[0] - (double)2.0) <= (double)1.0E-12);
  ASSERT(fabs(Tiny[0] - (double)2.0) <= (double)1.0E-12);
}

// A value without a sign terminates the search: the bracket cannot be updated
// with it, so the point is reported as having no answer.
TEST(FOANaNObjectiveEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)4.0}, Root[1];
  double       Low[1] = {(double)0.0}, High[1] = {(double)5.0};
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  // NaN at a pivot.
  BisectionRootFinding(1, A, B, NaNObjective, MaxIterations, Tolerance, Root,
                       NULL);
  ASSERT(isnan(Root[0]));

  // NaN at a midpoint.
  BisectionRootFinding(1, Low, High, GapNaNObjective, MaxIterations, Tolerance,
                       Root, NULL);
  ASSERT(isnan(Root[0]));
}

// The result buffer must not overlap a pivot: the NaN fill runs before the
// pivots are read, so an overlapping buffer is poisoned and the call reports
// the absence of an answer. There is no Fortran counterpart, because the
// Fortran result is a new array and cannot overlap the pivots.
TEST(FOAOverlappingRootEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)5.0};
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  ResetRecording();

  // Root aliases the left pivot.
  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations, Tolerance, A,
                       NULL);

  ASSERT(isnan(A[0]));
  ASSERT(RecordedCount == 2);

  // Root aliases the right pivot.
  A[0] = (double)0.0;
  B[0] = (double)5.0;

  ResetRecording();

  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations, Tolerance, B,
                       NULL);

  ASSERT(isnan(B[0]));
  ASSERT(RecordedCount == 2);
}

// A NaN pivot must not be mistaken for a bracket: it is the absence of a sign.
TEST(FOANaNPivotEdgeCase) {
  double       A[1] = {(double)NAN}, B[1] = {(double)4.0}, Root[1];
  const size_t MaxIterations = 100;
  const double Tolerance     = (double)0.0;

  BisectionRootFinding(1, A, B, LineObjective, MaxIterations, Tolerance, Root,
                       NULL);

  ASSERT(isnan(Root[0]));
}

// Pivots on the same side of zero do not bracket it: the routine reports the
// absence of an answer instead of returning a point which is not one.
TEST(FOAUnbracketedPivotEdgeCase) {
  double       A[1] = {(double)2.0}, B[1] = {(double)5.0}, Root[1];
  double       Low[1] = {(double)0.0}, High[1] = {(double)4.0};
  const size_t MaxIterations = 100;

  BisectionRootFinding(1, A, B, QuadraticObjective, MaxIterations, 0.0, Root,
                       NULL);
  ASSERT(isnan(Root[0]));

  BisectionRootFinding(1, Low, High, ConstantObjective, MaxIterations, 0.0,
                       Root, NULL);
  ASSERT(isnan(Root[0]));
}

// A negative residual bound can never be met, so the call is rejected before
// the objective is evaluated.
TEST(FOANegativeToleranceEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)3.0}, Root[1];
  const size_t MaxIterations = 100;

  ResetRecording();

  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations,
                       (double)-1.0E-6, Root, NULL);

  ASSERT(RecordedCount == 0);
  ASSERT(isnan(Root[0]));
}

// A NaN residual bound cannot be compared with, so the call is rejected.
TEST(FOANaNToleranceEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)3.0}, Root[1];
  const size_t MaxIterations = 100;

  ResetRecording();

  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations, (double)NAN,
                       Root, NULL);

  ASSERT(RecordedCount == 0);
  ASSERT(isnan(Root[0]));
}

// A cap of zero leaves no iteration to perform, so the call is rejected.
TEST(FOAZeroIterationsEdgeCase) {
  double A[1] = {(double)0.0}, B[1] = {(double)3.0}, Root[1];

  ResetRecording();

  BisectionRootFinding(1, A, B, RecordingObjective, 0, 0.0, Root, NULL);

  ASSERT(RecordedCount == 0);
  ASSERT(isnan(Root[0]));
}

// An empty space has no component to write: the routine is a no-op, and the
// caller buffer is left as it was.
TEST(FOAEmptySpaceEdgeCase) {
  double Root[1] = {(double)42.0};

  ResetRecording();

  BisectionRootFinding(0, NULL, NULL, NULL, 100, 0.0, Root, NULL);

  ASSERT(RecordedCount == 0);
  ASSERT(Root[0] == (double)42.0);
}

// A NULL pivot is a rejection: the routine reports it instead of reading an
// address the caller does not own.
TEST(FOANullPivotEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)3.0}, Root[1];
  const size_t MaxIterations = 100;

  ResetRecording();

  BisectionRootFinding(1, NULL, B, RecordingObjective, MaxIterations, 0.0, Root,
                       NULL);
  ASSERT(isnan(Root[0]));

  BisectionRootFinding(1, A, NULL, RecordingObjective, MaxIterations, 0.0, Root,
                       NULL);
  ASSERT(isnan(Root[0]));

  ASSERT(RecordedCount == 0);
}

// A NULL objective is a rejection: there is nothing to evaluate.
TEST(FOANullObjectiveEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)3.0}, Root[1];
  const size_t MaxIterations = 100;

  BisectionRootFinding(1, A, B, NULL, MaxIterations, 0.0, Root, NULL);

  ASSERT(isnan(Root[0]));
}

// A NULL result buffer is a rejection before anything is evaluated.
TEST(FOANullRootEdgeCase) {
  double       A[1] = {(double)0.0}, B[1] = {(double)3.0};
  const size_t MaxIterations = 100;

  ResetRecording();

  BisectionRootFinding(1, A, B, RecordingObjective, MaxIterations, 0.0, NULL,
                       NULL);

  ASSERT(RecordedCount == 0);
}

TEST_SUITE(FOACFunctionalEdgeCaseSuite,
           FOAExactZeroAtPivotEdgeCase,
           FOAExactZeroAtMidpointEdgeCase,
           FOACollapsedBracketEdgeCase,
           FOAIterationCapAccuracyEdgeCase,
           FOASignOnlyObjectiveEdgeCase,
           FOAScaledSignObjectiveEdgeCase,
           FOANaNObjectiveEdgeCase);

TEST_SUITE(FOACStructuralSuite,
           FOAOverlappingRootEdgeCase,
           FOANaNPivotEdgeCase,
           FOAUnbracketedPivotEdgeCase,
           FOANegativeToleranceEdgeCase,
           FOANaNToleranceEdgeCase,
           FOAZeroIterationsEdgeCase,
           FOAEmptySpaceEdgeCase,
           FOANullPivotEdgeCase,
           FOANullObjectiveEdgeCase,
           FOANullRootEdgeCase);
