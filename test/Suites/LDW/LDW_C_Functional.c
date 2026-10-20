#include "src/LDW/LapackDeferredWrappers.h"
#include "test/TSD/TSD.h"
#include <complex.h>
#include <float.h>
#include <limits.h>
#include <math.h>

extern void zheevd_(void);

void DummyZHEEVD(void) {
  // Dummy.
}

TEST(LDWDHZeroCount) {
  double complex Matrix[1], Unitary[1];
  double         Eigenvalues[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  DiagHermitian((size_t)0, Matrix, Unitary, Eigenvalues, DummyZHEEVD, &Info);
  ASSERT(Info == 0);
}

TEST(LDWDHNullMatrix) {
  double complex Unitary[1];
  double         Eigenvalues[1];
  int            Info = 1;
  DiagHermitian((size_t)0, NULL, Unitary, Eigenvalues, DummyZHEEVD, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWDHNullUnitary) {
  double complex Matrix[1];
  double         Eigenvalues[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  DiagHermitian((size_t)0, Matrix, NULL, Eigenvalues, DummyZHEEVD, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWDHNullEigenvalues) {
  double complex Matrix[1], Unitary[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  DiagHermitian((size_t)0, Matrix, Unitary, NULL, DummyZHEEVD, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWDHNullZHEEVDProcedure) {
  double complex Matrix[1], Unitary[1];
  double         Eigenvalues[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  DiagHermitian((size_t)0, Matrix, Unitary, Eigenvalues, NULL, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWDHNullInfo) {
  double complex Matrix[1], Unitary[1];
  double         Eigenvalues[1];
  Matrix[0] = 0.0;
  DiagHermitian((size_t)0, Matrix, Unitary, Eigenvalues, DummyZHEEVD, NULL);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
TEST(LDWDHTooLargeMatrix) {
  double complex Matrix[1], Unitary[1];
  double         Eigenvalues[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  DiagHermitian((size_t)(INT_MAX) + (size_t)1, Matrix, Unitary, Eigenvalues,
                DummyZHEEVD, &Info);
  ASSERT(Info == INT_MAX);
}
#pragma GCC diagnostic pop

TEST_SUITE(LDWDHEdgeCaseSuite,
           LDWDHZeroCount,
           LDWDHNullMatrix,
           LDWDHNullUnitary,
           LDWDHNullEigenvalues,
           LDWDHNullZHEEVDProcedure,
           LDWDHNullInfo,
           LDWDHTooLargeMatrix);

TEST(LDWDegenZeroCount) {
  double E[1] = {0.0};
  size_t D[1];
  D[0] = 12;
  DegeneracyGroup((size_t)0, E, (double)0.1, D);
  ASSERT(D[0] == 12);
}

TEST(LDWDegenNegativeThr) {
  double E[1] = {0.0};
  size_t D[1];
  D[0] = 12;
  DegeneracyGroup((size_t)1, E, (double)-0.1, D);
  ASSERT(D[0] == 12);
}

TEST(LDWDegenNullEig) {
  size_t D[1];
  D[0] = 12;
  DegeneracyGroup((size_t)1, NULL, (double)0.1, D);
  ASSERT(D[0] == 12);
}

TEST(LDWDegenNullDeg) {
  double E[1] = {0.0};
  DegeneracyGroup((size_t)1, E, (double)0.1, NULL);
}

TEST_SUITE(LDWDegenEdgeCaseSuite,
           LDWDegenZeroCount,
           LDWDegenNegativeThr,
           LDWDegenNullEig,
           LDWDegenNullDeg);

static double complex ColumnDot(const size_t          N,
                                const double complex *U,
                                const size_t          i,
                                const size_t          j) {
  double complex Dot = 0.0;
  size_t         k;

  for (k = 0; k < N; k++) {
    Dot += U[k + i * N] * conj(U[k + j * N]);
  }

  return Dot;
}

static void MultiplyMatrices(const size_t          N,
                             const double complex *A,
                             const double complex *B,
                             double complex       *C) {
  size_t i, j, k;

  for (j = 0; j < N; j++) {
    for (i = 0; i < N; i++) {
      C[i + j * N] = 0.0;
      for (k = 0; k < N; k++) {
        C[i + j * N] += A[i + k * N] * B[k + j * N];
      }
    }
  }
}

TEST(LDWDiagHermitianReference) {
  const size_t Dimension = 3;
  const double Tolerance = 10.0 * DBL_EPSILON;

  double complex Matrix[Dimension * Dimension], Unitary[Dimension * Dimension];
  double complex Adjoint[Dimension * Dimension], Product[Dimension * Dimension],
      Transformed[Dimension * Dimension];
  double Eigenvalues[Dimension];
  int    Info;

  size_t i, j;

  Matrix[0] = 3.0;      // (1, 1)
  Matrix[1] = -1.0 * I; // (2, 1)
  Matrix[2] = 1.0;      // (3, 1)
  Matrix[3] = 1.0 * I;  // (1, 2)
  Matrix[4] = 3.0;      // (2, 2)
  Matrix[5] = -1.0 * I; // (3, 2)
  Matrix[6] = 1.0;      // (1, 3)
  Matrix[7] = 1.0 * I;  // (2, 3)
  Matrix[8] = 3.0;      // (3, 3)

  DiagHermitian(Dimension, Matrix, Unitary, Eigenvalues, zheevd_, &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  ASSERT(fabs(Eigenvalues[0] - 1.0) < Tolerance);
  ASSERT(fabs(cabs(ColumnDot(Dimension, Unitary, 0, 0)) - 1.0) < Tolerance);
  ASSERT(cabs(ColumnDot(Dimension, Unitary, 0, 1)) < Tolerance);
  ASSERT(cabs(ColumnDot(Dimension, Unitary, 0, 2)) < Tolerance);

  ASSERT(fabs(Eigenvalues[1] - 4.0) < Tolerance);
  ASSERT(cabs(ColumnDot(Dimension, Unitary, 1, 0)) < Tolerance);
  ASSERT(fabs(cabs(ColumnDot(Dimension, Unitary, 1, 1)) - 1.0) < Tolerance);
  ASSERT(cabs(ColumnDot(Dimension, Unitary, 1, 2)) < Tolerance);

  ASSERT(fabs(Eigenvalues[2] - 4.0) < Tolerance);
  ASSERT(cabs(ColumnDot(Dimension, Unitary, 2, 0)) < Tolerance);
  ASSERT(cabs(ColumnDot(Dimension, Unitary, 2, 1)) < Tolerance);
  ASSERT(fabs(cabs(ColumnDot(Dimension, Unitary, 2, 2)) - 1.0) < Tolerance);

  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      Adjoint[i + j * Dimension] = conj(Unitary[j + i * Dimension]);
    }
  }

  MultiplyMatrices(Dimension, Matrix, Unitary, Product);
  MultiplyMatrices(Dimension, Adjoint, Product, Transformed);

  for (i = 0; i < Dimension; i++) {
    for (j = 0; j < Dimension; j++) {
      if (i == j) {
        ASSERT(cabs(Transformed[i + j * Dimension] - Eigenvalues[i]) <
               Tolerance);
      } else {
        ASSERT(cabs(Transformed[i + j * Dimension]) < Tolerance);
      }
    }
  }
}

TEST(LDWDegeneracyGroupReference) {
  double E[9] = {0.0, 0.2, 0.5, 0.9, 0.95, 1.1, 1.2, 7.1, 7.2};
  size_t D[9], i;
  size_t Ref[9] = {3, 0, 0, 4, 0, 0, 0, 2, 0};

  DegeneracyGroup((size_t)9, E, (double)0.3, D);
  for (i = 0; i < 9; i++) {
    ASSERT(D[i] == Ref[i]);
  }
}

TEST_SUITE(LDWNominalSuite,
           LDWDiagHermitianReference,
           LDWDegeneracyGroupReference);
