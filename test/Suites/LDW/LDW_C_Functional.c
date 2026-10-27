#include "src/LDW/LapackDeferredWrappers.h"
#include "test/TSD/TSD.h"
#include <complex.h>
#include <float.h>
#include <limits.h>
#include <math.h>

extern void zheevd_(void);
extern void zgees_(void);
extern void zgges_(void);
extern void zgesdd_(void);

void DummyZHEEVD(void) {
  // Dummy.
}

void DummyZGEES(void) {
  // Dummy.
}

void DummyZGESDD(void) {
  // Dummy.
}

void DummyZGGES(void) {
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
  // Eigenvalues is optional: a NULL array only skips that output.
  DiagHermitian((size_t)0, Matrix, Unitary, NULL, DummyZHEEVD, &Info);
  ASSERT(Info == 0);
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

TEST(LDWSZeroCount) {
  double complex Matrix[1], Unitary[1], SchurForm[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SchurDecomposition((size_t)0, Matrix, Unitary, SchurForm, DummyZGEES, &Info);
  ASSERT(Info == 0);
}

TEST(LDWSNullMatrix) {
  double complex Unitary[1], SchurForm[1];
  int            Info = 1;
  SchurDecomposition((size_t)0, NULL, Unitary, SchurForm, DummyZGEES, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWSNullUnitary) {
  double complex Matrix[1], SchurForm[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SchurDecomposition((size_t)0, Matrix, NULL, SchurForm, DummyZGEES, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWSNullSchurForm) {
  double complex Matrix[1], Unitary[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  // SchurForm is optional: a NULL array only skips that output.
  SchurDecomposition((size_t)0, Matrix, Unitary, NULL, DummyZGEES, &Info);
  ASSERT(Info == 0);
}

TEST(LDWSNullZGEESProcedure) {
  double complex Matrix[1], Unitary[1], SchurForm[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SchurDecomposition((size_t)0, Matrix, Unitary, SchurForm, NULL, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWSNullInfo) {
  double complex Matrix[1], Unitary[1], SchurForm[1];
  Matrix[0] = 0.0;
  SchurDecomposition((size_t)0, Matrix, Unitary, SchurForm, DummyZGEES, NULL);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
TEST(LDWSTooLargeMatrix) {
  double complex Matrix[1], Unitary[1], SchurForm[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SchurDecomposition((size_t)(INT_MAX) + (size_t)1, Matrix, Unitary, SchurForm,
                     DummyZGEES, &Info);
  ASSERT(Info == INT_MAX);
}
#pragma GCC diagnostic pop

TEST_SUITE(LDWSEdgeCaseSuite,
           LDWSZeroCount,
           LDWSNullMatrix,
           LDWSNullUnitary,
           LDWSNullSchurForm,
           LDWSNullZGEESProcedure,
           LDWSNullInfo,
           LDWSTooLargeMatrix);

TEST(LDWSVDZeroCount) {
  double complex Matrix[1], UnitaryU[1], UnitaryVH[1];
  double         DiagSigma[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SVDDecomposition((size_t)0, (size_t)0, Matrix, UnitaryU, DiagSigma, UnitaryVH,
                   DummyZGESDD, &Info);
  ASSERT(Info == 0);
}

TEST(LDWSVDZeroColumnCount) {
  double complex Matrix[1], UnitaryU[1], UnitaryVH[1];
  double         DiagSigma[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  // A single zero dimension is enough for the empty decomposition.
  SVDDecomposition((size_t)0, (size_t)3, Matrix, UnitaryU, DiagSigma, UnitaryVH,
                   DummyZGESDD, &Info);
  ASSERT(Info == 0);
}

TEST(LDWSVDNullMatrix) {
  double complex UnitaryU[1], UnitaryVH[1];
  double         DiagSigma[1];
  int            Info = 1;
  SVDDecomposition((size_t)0, (size_t)0, NULL, UnitaryU, DiagSigma, UnitaryVH,
                   DummyZGESDD, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWSVDNullUnitaryU) {
  double complex Matrix[1], UnitaryVH[1];
  double         DiagSigma[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SVDDecomposition((size_t)0, (size_t)0, Matrix, NULL, DiagSigma, UnitaryVH,
                   DummyZGESDD, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWSVDNullDiagSigma) {
  double complex Matrix[1], UnitaryU[1], UnitaryVH[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  // DiagSigma is optional: a NULL array only skips that output.
  SVDDecomposition((size_t)0, (size_t)0, Matrix, UnitaryU, NULL, UnitaryVH,
                   DummyZGESDD, &Info);
  ASSERT(Info == 0);
}

TEST(LDWSVDNullUnitaryVH) {
  double complex Matrix[1], UnitaryU[1];
  double         DiagSigma[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SVDDecomposition((size_t)0, (size_t)0, Matrix, UnitaryU, DiagSigma, NULL,
                   DummyZGESDD, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWSVDNullZGESDDProcedure) {
  double complex Matrix[1], UnitaryU[1], UnitaryVH[1];
  double         DiagSigma[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SVDDecomposition((size_t)0, (size_t)0, Matrix, UnitaryU, DiagSigma, UnitaryVH,
                   NULL, &Info);
  ASSERT(Info == 1); // Info unchanged.
}

TEST(LDWSVDNullInfo) {
  double complex Matrix[1], UnitaryU[1], UnitaryVH[1];
  double         DiagSigma[1];
  Matrix[0] = 0.0;
  SVDDecomposition((size_t)0, (size_t)0, Matrix, UnitaryU, DiagSigma, UnitaryVH,
                   DummyZGESDD, NULL);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
TEST(LDWSVDTooLargeRowCount) {
  double complex Matrix[1], UnitaryU[1], UnitaryVH[1];
  double         DiagSigma[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SVDDecomposition((size_t)(INT_MAX) + (size_t)1, (size_t)1, Matrix, UnitaryU,
                   DiagSigma, UnitaryVH, DummyZGESDD, &Info);
  ASSERT(Info == INT_MAX);
}

TEST(LDWSVDTooLargeColumnCount) {
  double complex Matrix[1], UnitaryU[1], UnitaryVH[1];
  double         DiagSigma[1];
  int            Info = 1;
  Matrix[0]           = 0.0;
  SVDDecomposition((size_t)1, (size_t)(INT_MAX) + (size_t)1, Matrix, UnitaryU,
                   DiagSigma, UnitaryVH, DummyZGESDD, &Info);
  ASSERT(Info == INT_MAX);
}
#pragma GCC diagnostic pop

TEST_SUITE(LDWSVDEdgeCaseSuite,
           LDWSVDZeroCount,
           LDWSVDZeroColumnCount,
           LDWSVDNullMatrix,
           LDWSVDNullUnitaryU,
           LDWSVDNullDiagSigma,
           LDWSVDNullUnitaryVH,
           LDWSVDNullZGESDDProcedure,
           LDWSVDNullInfo,
           LDWSVDTooLargeRowCount,
           LDWSVDTooLargeColumnCount);

TEST(LDWGSZeroCount) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, EigenvaluesB, UnitaryVH,
                                DummyZGGES, &Info);
  ASSERT(Info == 0);
}

TEST(LDWGSZeroCountNoEigenvalues) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1], UnitaryVH[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  // Both eigenvalue arrays are optional and may be omitted together.
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, UnitaryU, NULL,
                                NULL, UnitaryVH, DummyZGGES, &Info);
  ASSERT(Info == 0);
}

TEST(LDWGSZeroCountEigenvaluesA) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesA[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  // The denominator output is optional on its own.
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, NULL, UnitaryVH, DummyZGGES,
                                &Info);
  ASSERT(Info == 0);
}

TEST(LDWGSZeroCountEigenvaluesB) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesB[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  // The numerator output is optional on its own.
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, UnitaryU, NULL,
                                EigenvaluesB, UnitaryVH, DummyZGGES, &Info);
  ASSERT(Info == 0);
}

TEST(LDWGSZeroCountNullUnitaryU) {
  double complex MatrixA[1], MatrixB[1], UnitaryVH[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  UnitaryVH[0]        = 42.0;
  EigenvaluesA[0]     = 42.0;
  EigenvaluesB[0]     = 42.0;
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, NULL, EigenvaluesA,
                                EigenvaluesB, UnitaryVH, DummyZGGES, &Info);
  ASSERT(Info == 1); // Info unchanged.
  // A rejected call leaves every output untouched.
  ASSERT(UnitaryVH[0] == 42.0);
  ASSERT(EigenvaluesA[0] == 42.0);
  ASSERT(EigenvaluesB[0] == 42.0);
}

TEST(LDWGSZeroCountNullMatrixA) {
  double complex MatrixB[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  int            Info = 1;
  MatrixB[0]          = 0.0;
  UnitaryU[0]         = 42.0;
  UnitaryVH[0]        = 42.0;
  EigenvaluesA[0]     = 42.0;
  EigenvaluesB[0]     = 42.0;
  GeneralizedSchurDecomposition((size_t)0, NULL, MatrixB, UnitaryU,
                                EigenvaluesA, EigenvaluesB, UnitaryVH,
                                DummyZGGES, &Info);
  ASSERT(Info == 1); // Info unchanged.
  // A rejected call leaves every output untouched.
  ASSERT(UnitaryU[0] == 42.0);
  ASSERT(UnitaryVH[0] == 42.0);
  ASSERT(EigenvaluesA[0] == 42.0);
  ASSERT(EigenvaluesB[0] == 42.0);
}

TEST(LDWGSZeroCountNullMatrixB) {
  double complex MatrixA[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  UnitaryU[0]         = 42.0;
  UnitaryVH[0]        = 42.0;
  EigenvaluesA[0]     = 42.0;
  EigenvaluesB[0]     = 42.0;
  GeneralizedSchurDecomposition((size_t)0, MatrixA, NULL, UnitaryU,
                                EigenvaluesA, EigenvaluesB, UnitaryVH,
                                DummyZGGES, &Info);
  ASSERT(Info == 1); // Info unchanged.
  // A rejected call leaves every output untouched.
  ASSERT(UnitaryU[0] == 42.0);
  ASSERT(UnitaryVH[0] == 42.0);
  ASSERT(EigenvaluesA[0] == 42.0);
  ASSERT(EigenvaluesB[0] == 42.0);
}

TEST(LDWGSZeroCountNullUnitaryVH) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  UnitaryU[0]         = 42.0;
  EigenvaluesA[0]     = 42.0;
  EigenvaluesB[0]     = 42.0;
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, EigenvaluesB, NULL, DummyZGGES,
                                &Info);
  ASSERT(Info == 1); // Info unchanged.
  // A rejected call leaves every output untouched.
  ASSERT(UnitaryU[0] == 42.0);
  ASSERT(EigenvaluesA[0] == 42.0);
  ASSERT(EigenvaluesB[0] == 42.0);
}

TEST(LDWGSZeroCountNullZGGESProcedure) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  UnitaryU[0]         = 42.0;
  UnitaryVH[0]        = 42.0;
  EigenvaluesA[0]     = 42.0;
  EigenvaluesB[0]     = 42.0;
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, EigenvaluesB, UnitaryVH, NULL,
                                &Info);
  ASSERT(Info == 1); // Info unchanged.
  // A rejected call leaves every output untouched.
  ASSERT(UnitaryU[0] == 42.0);
  ASSERT(UnitaryVH[0] == 42.0);
  ASSERT(EigenvaluesA[0] == 42.0);
  ASSERT(EigenvaluesB[0] == 42.0);
}

TEST(LDWGSZeroCountNullInfo) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  MatrixA[0] = 0.0;
  MatrixB[0] = 0.0;
  GeneralizedSchurDecomposition((size_t)0, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, EigenvaluesB, UnitaryVH,
                                DummyZGGES, NULL);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
TEST(LDWGSTooLargeMatrix) {
  double complex MatrixA[1], MatrixB[1], UnitaryU[1], UnitaryVH[1];
  double complex EigenvaluesA[1], EigenvaluesB[1];
  int            Info = 1;
  MatrixA[0]          = 0.0;
  MatrixB[0]          = 0.0;
  UnitaryU[0]         = 42.0;
  UnitaryVH[0]        = 42.0;
  EigenvaluesA[0]     = 42.0;
  EigenvaluesB[0]     = 42.0;
  GeneralizedSchurDecomposition((size_t)(INT_MAX) + (size_t)1, MatrixA, MatrixB,
                                UnitaryU, EigenvaluesA, EigenvaluesB, UnitaryVH,
                                DummyZGGES, &Info);
  ASSERT(Info == INT_MAX);
  // The rejected dimension leaves every output untouched.
  ASSERT(UnitaryU[0] == 42.0);
  ASSERT(UnitaryVH[0] == 42.0);
  ASSERT(EigenvaluesA[0] == 42.0);
  ASSERT(EigenvaluesB[0] == 42.0);
}
#pragma GCC diagnostic pop

TEST_SUITE(LDWGSEdgeCaseSuite,
           LDWGSZeroCount,
           LDWGSZeroCountNoEigenvalues,
           LDWGSZeroCountEigenvaluesA,
           LDWGSZeroCountEigenvaluesB,
           LDWGSZeroCountNullUnitaryU,
           LDWGSZeroCountNullMatrixA,
           LDWGSZeroCountNullMatrixB,
           LDWGSZeroCountNullUnitaryVH,
           LDWGSZeroCountNullZGGESProcedure,
           LDWGSZeroCountNullInfo,
           LDWGSTooLargeMatrix);

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

TEST(LDWSchurReference) {
  const size_t Dimension = 3;
  const double Tolerance = 10.0 * DBL_EPSILON;

  double complex Matrix[Dimension * Dimension], Original[Dimension * Dimension];
  double complex Unitary[Dimension * Dimension],
      SchurForm[Dimension * Dimension];
  double complex Adjoint[Dimension * Dimension], Product[Dimension * Dimension],
      Transformed[Dimension * Dimension];
  double complex Eigenvalues[Dimension];
  double complex Expected[Dimension];
  int            Used[Dimension];
  int            Info, Found;
  size_t         i, j;

  // Cyclic permutation matrix. It is unitary (hence normal), so its Schur
  // form is diagonal, with the cube roots of unity on the diagonal.
  Matrix[0] = 0.0; // (1, 1)
  Matrix[1] = 1.0; // (2, 1)
  Matrix[2] = 0.0; // (3, 1)
  Matrix[3] = 0.0; // (1, 2)
  Matrix[4] = 0.0; // (2, 2)
  Matrix[5] = 1.0; // (3, 2)
  Matrix[6] = 1.0; // (1, 3)
  Matrix[7] = 0.0; // (2, 3)
  Matrix[8] = 0.0; // (3, 3)

  for (i = 0; i < Dimension * Dimension; i++) {
    Original[i] = Matrix[i];
  }

  SchurDecomposition(Dimension, Matrix, Unitary, SchurForm, zgees_, &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  // Matrix is an input-only argument and must not be overwritten.
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Matrix[i] - Original[i]) <= 0.0);
  }

  // The columns of Unitary are orthonormal.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(ColumnDot(Dimension, Unitary, i, j)) - 1.0) <
               Tolerance);
      } else {
        ASSERT(cabs(ColumnDot(Dimension, Unitary, i, j)) < Tolerance);
      }
    }
  }

  // A normal matrix has a diagonal Schur form: every off-diagonal entry must
  // vanish and every diagonal entry must lie on the unit circle.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(SchurForm[i + j * Dimension]) - 1.0) < Tolerance);
      } else {
        ASSERT(cabs(SchurForm[i + j * Dimension]) < Tolerance);
      }
    }
  }

  // SchurForm = U^H * Matrix * U.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      Adjoint[i + j * Dimension] = conj(Unitary[j + i * Dimension]);
    }
  }

  MultiplyMatrices(Dimension, Matrix, Unitary, Product);
  MultiplyMatrices(Dimension, Adjoint, Product, Transformed);

  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Transformed[i] - SchurForm[i]) < Tolerance);
  }

  // Matrix * U = U * SchurForm.
  MultiplyMatrices(Dimension, Matrix, Unitary, Product);
  MultiplyMatrices(Dimension, Unitary, SchurForm, Transformed);

  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Product[i] - Transformed[i]) < Tolerance);
  }

  // The diagonal of the Schur form holds the cube roots of unity. Their
  // order is implementation-defined, so match them as a set.
  for (i = 0; i < Dimension; i++) {
    Eigenvalues[i] = SchurForm[i + i * Dimension];
    Used[i]        = 0;
  }

  Expected[0] = 1.0;
  Expected[1] = -0.5 + (sqrt(3.0) / 2.0) * I;
  Expected[2] = -0.5 - (sqrt(3.0) / 2.0) * I;

  for (i = 0; i < Dimension; i++) {
    Found = 0;
    for (j = 0; j < Dimension; j++) {
      if (!Used[j] && cabs(Eigenvalues[j] - Expected[i]) < Tolerance) {
        Used[j] = 1;
        Found   = 1;
        break;
      }
    }
    ASSERT(Found);
  }
}

// An arbitrary dense matrix: it is neither normal, nor unitary, nor
// triangular, so its Schur form is triangular without being diagonal. The
// test proves the defining identity SchurForm = U^H * Matrix * U.
TEST(LDWSchurCongruence) {
  const size_t Dimension = 3;
  // The residual is bounded by eps times the Frobenius norm of Matrix.
  const double Tolerance = 100.0 * DBL_EPSILON * 7.5;

  double complex Matrix[Dimension * Dimension], Original[Dimension * Dimension],
      Unitary[Dimension * Dimension], SchurForm[Dimension * Dimension];
  double complex Adjoint[Dimension * Dimension], Product[Dimension * Dimension],
      Transformed[Dimension * Dimension];
  double complex Eigenvalues[Dimension], Expected[Dimension];
  double         Magnitude, StrictUpper;
  int            Used[Dimension];
  int            Info, Found;
  size_t         i, j;

  // The spectrum is well separated: 1 - 2i, 3 + 3i and 2i.
  Matrix[0] = 1.0 + 2.0 * I;  // (1, 1)
  Matrix[1] = -2.0 + 1.0 * I; // (2, 1)
  Matrix[2] = 1.0 - 1.0 * I;  // (3, 1)
  Matrix[3] = 3.0 - 1.0 * I;  // (1, 2)
  Matrix[4] = 4.0;            // (2, 2)
  Matrix[5] = 2.0 * I;        // (3, 2)
  Matrix[6] = 1.0;            // (1, 3)
  Matrix[7] = 1.0 + 3.0 * I;  // (2, 3)
  Matrix[8] = -1.0 + 1.0 * I; // (3, 3)

  for (i = 0; i < Dimension * Dimension; i++) {
    Original[i] = Matrix[i];
  }

  SchurDecomposition(Dimension, Matrix, Unitary, SchurForm, zgees_, &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  // Matrix is an input-only argument and must not be overwritten.
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Matrix[i] - Original[i]) <= 0.0);
  }

  // The columns of Unitary are orthonormal.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(ColumnDot(Dimension, Unitary, i, j)) - 1.0) <
               Tolerance);
      } else {
        ASSERT(cabs(ColumnDot(Dimension, Unitary, i, j)) < Tolerance);
      }
    }
  }

  // SchurForm is upper triangular.
  for (j = 0; j < Dimension; j++) {
    for (i = j + 1; i < Dimension; i++) {
      ASSERT(cabs(SchurForm[i + j * Dimension]) < Tolerance);
    }
  }

  // Matrix is not normal, so the strictly upper triangle cannot vanish: its
  // Frobenius norm is sqrt(||Matrix||_F^2 - sum |lambda|^2) = sqrt(28).
  StrictUpper = 0.0;
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < j; i++) {
      Magnitude = cabs(SchurForm[i + j * Dimension]);
      StrictUpper += Magnitude * Magnitude;
    }
  }
  ASSERT(sqrt(StrictUpper) > 5.0);

  // SchurForm = U^H * Matrix * U, entry by entry.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      Adjoint[i + j * Dimension] = conj(Unitary[j + i * Dimension]);
    }
  }

  MultiplyMatrices(Dimension, Matrix, Unitary, Product);
  MultiplyMatrices(Dimension, Adjoint, Product, Transformed);

  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Transformed[i] - SchurForm[i]) < Tolerance);
  }

  // Matrix * U = U * SchurForm.
  MultiplyMatrices(Dimension, Matrix, Unitary, Product);
  MultiplyMatrices(Dimension, Unitary, SchurForm, Transformed);

  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Product[i] - Transformed[i]) < Tolerance);
  }

  // The diagonal of the Schur form holds the spectrum of Matrix. Its order is
  // implementation-defined, so match the values as a set.
  for (i = 0; i < Dimension; i++) {
    Eigenvalues[i] = SchurForm[i + i * Dimension];
    Used[i]        = 0;
  }

  Expected[0] = 1.0 - 2.0 * I;
  Expected[1] = 3.0 + 3.0 * I;
  Expected[2] = 2.0 * I;

  for (i = 0; i < Dimension; i++) {
    Found = 0;
    for (j = 0; j < Dimension; j++) {
      if (!Used[j] && cabs(Eigenvalues[j] - Expected[i]) < Tolerance) {
        Used[j] = 1;
        Found   = 1;
        break;
      }
    }
    ASSERT(Found);
  }
}

// C = A * B for a general M-by-K times K-by-N product.
static void MultiplyMatricesGeneral(const size_t          M,
                                    const size_t          N,
                                    const size_t          K,
                                    const double complex *A,
                                    const double complex *B,
                                    double complex       *C) {
  size_t i, j, k;

  for (j = 0; j < N; j++) {
    for (i = 0; i < M; i++) {
      C[i + j * M] = 0.0;
      for (k = 0; k < K; k++) {
        C[i + j * M] += A[i + k * M] * B[k + j * K];
      }
    }
  }
}

// An arbitrary dense matrix: the singular values are not available in closed
// form, so the singular values, the two factors and their product are checked
// against structural invariants instead.
TEST(LDWSVDReference) {
  const size_t Dimension = 3;
  const double Tolerance = 100.0 * DBL_EPSILON * 7.5;

  double complex Matrix[Dimension * Dimension], Original[Dimension * Dimension];
  double complex UnitaryU[Dimension * Dimension],
      UnitaryVH[Dimension * Dimension];
  double complex Sigma[Dimension * Dimension], Scaled[Dimension * Dimension],
      Product[Dimension * Dimension];
  double DiagSigma[Dimension], Frobenius, SigmaSum;
  int    Info;
  size_t i, j;

  Matrix[0] = 1.0 + 2.0 * I;  // (1, 1)
  Matrix[1] = -2.0 + 1.0 * I; // (2, 1)
  Matrix[2] = 1.0 - 1.0 * I;  // (3, 1)
  Matrix[3] = 3.0 - 1.0 * I;  // (1, 2)
  Matrix[4] = 4.0;            // (2, 2)
  Matrix[5] = 2.0 * I;        // (3, 2)
  Matrix[6] = 1.0;            // (1, 3)
  Matrix[7] = 1.0 + 3.0 * I;  // (2, 3)
  Matrix[8] = -1.0 + 1.0 * I; // (3, 3)

  for (i = 0; i < Dimension * Dimension; i++) {
    Original[i] = Matrix[i];
  }

  SVDDecomposition(Dimension, Dimension, Matrix, UnitaryU, DiagSigma, UnitaryVH,
                   zgesdd_, &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  // Matrix is an input-only argument and must not be overwritten.
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Matrix[i] - Original[i]) <= 0.0);
  }

  // The singular values are real, non-negative and in ascending order.
  ASSERT(DiagSigma[0] >= 0.0);
  ASSERT(DiagSigma[0] <= DiagSigma[1]);
  ASSERT(DiagSigma[1] <= DiagSigma[2]);

  // Both factors are unitary: their columns are orthonormal.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(ColumnDot(Dimension, UnitaryU, i, j)) - 1.0) <
               Tolerance);
        ASSERT(fabs(cabs(ColumnDot(Dimension, UnitaryVH, i, j)) - 1.0) <
               Tolerance);
      } else {
        ASSERT(cabs(ColumnDot(Dimension, UnitaryU, i, j)) < Tolerance);
        ASSERT(cabs(ColumnDot(Dimension, UnitaryVH, i, j)) < Tolerance);
      }
    }
  }

  // The squared singular values add up to the squared Frobenius norm.
  Frobenius = 0.0;
  SigmaSum  = 0.0;
  for (i = 0; i < Dimension * Dimension; i++) {
    Frobenius += pow(cabs(Original[i]), 2.0);
  }
  for (i = 0; i < Dimension; i++) {
    SigmaSum += DiagSigma[i] * DiagSigma[i];
  }
  ASSERT(fabs(SigmaSum - Frobenius) < Tolerance * (1.0 + Frobenius));

  // Congruence: Matrix = U * diag(DiagSigma) * VH.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      Sigma[i + j * Dimension] = (i == j) ? DiagSigma[i] : 0.0;
    }
  }

  MultiplyMatricesGeneral(Dimension, Dimension, Dimension, UnitaryU, Sigma,
                          Scaled);
  MultiplyMatricesGeneral(Dimension, Dimension, Dimension, Scaled, UnitaryVH,
                          Product);

  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Product[i] - Original[i]) < Tolerance);
  }
}

// A tall matrix whose singular values are known: the two columns are
// orthogonal and have squared norms 10 and 20, so the singular values are
// sqrt(10) and sqrt(20). Their ascending order is therefore predictable, and
// the unused columns of the square factor U are part of the congruence.
TEST(LDWSVDCongruenceTall) {
  const size_t Rows      = 4;
  const size_t Columns   = 2;
  const double Tolerance = 100.0 * DBL_EPSILON * 8.0;

  double complex Matrix[Rows * Columns], Original[Rows * Columns];
  double complex UnitaryU[Rows * Rows], UnitaryVH[Columns * Columns];
  double complex Sigma[Rows * Columns], Scaled[Rows * Columns],
      Product[Rows * Columns];
  double DiagSigma[Columns];
  int    Info;
  size_t i, j;

  for (i = 0; i < Rows * Columns; i++) {
    Matrix[i] = 0.0;
  }

  Matrix[0 + 0 * Rows] = 1.0; // (1, 1)
  Matrix[1 + 0 * Rows] = 0.0; // (2, 1)
  Matrix[2 + 0 * Rows] = 3.0; // (3, 1)
  Matrix[3 + 0 * Rows] = 0.0; // (4, 1)
  Matrix[0 + 1 * Rows] = 0.0; // (1, 2)
  Matrix[1 + 1 * Rows] = 2.0; // (2, 2)
  Matrix[2 + 1 * Rows] = 0.0; // (3, 2)
  Matrix[3 + 1 * Rows] = 4.0; // (4, 2)

  for (i = 0; i < Rows * Columns; i++) {
    Original[i] = Matrix[i];
  }

  SVDDecomposition(Rows, Columns, Matrix, UnitaryU, DiagSigma, UnitaryVH,
                   zgesdd_, &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  for (i = 0; i < Rows * Columns; i++) {
    ASSERT(cabs(Matrix[i] - Original[i]) <= 0.0);
  }

  ASSERT(fabs(DiagSigma[0] - sqrt(10.0)) < Tolerance);
  ASSERT(fabs(DiagSigma[1] - sqrt(20.0)) < Tolerance);

  for (j = 0; j < Rows; j++) {
    for (i = 0; i < Rows; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(ColumnDot(Rows, UnitaryU, i, j)) - 1.0) < Tolerance);
      } else {
        ASSERT(cabs(ColumnDot(Rows, UnitaryU, i, j)) < Tolerance);
      }
    }
  }

  for (j = 0; j < Columns; j++) {
    for (i = 0; i < Columns; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(ColumnDot(Columns, UnitaryVH, i, j)) - 1.0) <
               Tolerance);
      } else {
        ASSERT(cabs(ColumnDot(Columns, UnitaryVH, i, j)) < Tolerance);
      }
    }
  }

  for (i = 0; i < Rows * Columns; i++) {
    Sigma[i] = 0.0;
  }
  for (i = 0; i < Columns; i++) {
    Sigma[i + i * Rows] = DiagSigma[i];
  }

  MultiplyMatricesGeneral(Rows, Columns, Rows, UnitaryU, Sigma, Scaled);
  MultiplyMatricesGeneral(Rows, Columns, Columns, Scaled, UnitaryVH, Product);

  for (i = 0; i < Rows * Columns; i++) {
    ASSERT(cabs(Product[i] - Original[i]) < Tolerance);
  }
}

// Triangular pair with the known generalized eigenvalues 2, 3/2 and 4/3.
// Both matrices have non-zero sub-diagonal entries, so ZGGES must actually
// reduce the pair to upper triangular Schur forms.
TEST(LDWGeneralizedSchurReference) {
  const size_t Dimension = 3;
  const double Tolerance = 1000.0 * DBL_EPSILON;

  double complex MatrixA[Dimension * Dimension];
  double complex MatrixB[Dimension * Dimension];
  double complex AOriginal[Dimension * Dimension];
  double complex BOriginal[Dimension * Dimension];
  double complex UnitaryU[Dimension * Dimension];
  double complex UnitaryVH[Dimension * Dimension];
  double complex V[Dimension * Dimension];
  double complex EigenvaluesA[Dimension], EigenvaluesB[Dimension];
  double complex AdjointU[Dimension * Dimension];
  double complex Product[Dimension * Dimension];
  double complex Transformed[Dimension * Dimension];
  double complex S[Dimension * Dimension], T[Dimension * Dimension];
  double complex Expected[Dimension];
  int            Used[Dimension];
  int            Info, Found;
  size_t         i, j;

  MatrixA[0] = 2.0; // (1, 1)
  MatrixA[1] = 1.0; // (2, 1)
  MatrixA[2] = 4.0; // (3, 1)
  MatrixA[3] = 0.0; // (1, 2)
  MatrixA[4] = 3.0; // (2, 2)
  MatrixA[5] = 5.0; // (3, 2)
  MatrixA[6] = 0.0; // (1, 3)
  MatrixA[7] = 0.0; // (2, 3)
  MatrixA[8] = 4.0; // (3, 3)

  MatrixB[0] = 1.0; // (1, 1)
  MatrixB[1] = 2.0; // (2, 1)
  MatrixB[2] = 1.0; // (3, 1)
  MatrixB[3] = 0.0; // (1, 2)
  MatrixB[4] = 2.0; // (2, 2)
  MatrixB[5] = 1.0; // (3, 2)
  MatrixB[6] = 0.0; // (1, 3)
  MatrixB[7] = 0.0; // (2, 3)
  MatrixB[8] = 3.0; // (3, 3)

  for (i = 0; i < Dimension * Dimension; i++) {
    AOriginal[i] = MatrixA[i];
    BOriginal[i] = MatrixB[i];
  }

  GeneralizedSchurDecomposition(Dimension, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, EigenvaluesB, UnitaryVH, zgges_,
                                &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  // MatrixA and MatrixB are input-only arguments.
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(MatrixA[i] - AOriginal[i]) <= 0.0);
    ASSERT(cabs(MatrixB[i] - BOriginal[i]) <= 0.0);
  }

  // The columns of U and of V are orthonormal.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      V[i + j * Dimension] = conj(UnitaryVH[j + i * Dimension]);
    }
  }

  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(ColumnDot(Dimension, UnitaryU, i, j)) - 1.0) <
               Tolerance);
        ASSERT(fabs(cabs(ColumnDot(Dimension, V, i, j)) - 1.0) < Tolerance);
      } else {
        ASSERT(cabs(ColumnDot(Dimension, UnitaryU, i, j)) < Tolerance);
        ASSERT(cabs(ColumnDot(Dimension, V, i, j)) < Tolerance);
      }
    }
  }

  // S = U^H*A*V and T = U^H*B*V.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      AdjointU[i + j * Dimension] = conj(UnitaryU[j + i * Dimension]);
    }
  }

  MultiplyMatrices(Dimension, AOriginal, V, Product);
  MultiplyMatrices(Dimension, AdjointU, Product, S);
  MultiplyMatrices(Dimension, BOriginal, V, Product);
  MultiplyMatrices(Dimension, AdjointU, Product, T);

  // Both Schur forms are upper triangular; diag(T) is real and non-negative.
  for (j = 0; j < Dimension; j++) {
    for (i = j + 1; i < Dimension; i++) {
      ASSERT(cabs(S[i + j * Dimension]) < Tolerance);
      ASSERT(cabs(T[i + j * Dimension]) < Tolerance);
    }
    ASSERT(fabs(cimag(T[j + j * Dimension])) < Tolerance);
    ASSERT(creal(T[j + j * Dimension]) >= 0.0);
  }

  // A = U*S*V^H and B = U*T*V^H.
  MultiplyMatrices(Dimension, UnitaryU, S, Product);
  MultiplyMatrices(Dimension, Product, UnitaryVH, Transformed);
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Transformed[i] - AOriginal[i]) < Tolerance);
  }

  MultiplyMatrices(Dimension, UnitaryU, T, Product);
  MultiplyMatrices(Dimension, Product, UnitaryVH, Transformed);
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Transformed[i] - BOriginal[i]) < Tolerance);
  }

  // The returned pairs reproduce the known generalized eigenvalues 2, 3/2
  // and 4/3. Their order is implementation-defined, so match them as a set.
  Expected[0] = 2.0;
  Expected[1] = 1.5;
  Expected[2] = 4.0 / 3.0;

  for (i = 0; i < Dimension; i++) {
    EigenvaluesA[i] = EigenvaluesA[i] / EigenvaluesB[i];
    Used[i]         = 0;
  }

  for (i = 0; i < Dimension; i++) {
    Found = 0;
    for (j = 0; j < Dimension; j++) {
      if (!Used[j] && cabs(EigenvaluesA[j] - Expected[i]) < Tolerance) {
        Used[j] = 1;
        Found   = 1;
        break;
      }
    }
    ASSERT(Found);
  }
}

// An arbitrary dense pair: neither matrix is triangular or normal, so both
// Schur forms have a non-trivial upper triangle. The test proves the
// defining identities S = U^H*A*V and T = U^H*B*V.
TEST(LDWGeneralizedSchurCongruence) {
  const size_t Dimension = 3;
  const double Tolerance = 1000.0 * DBL_EPSILON;

  double complex MatrixA[Dimension * Dimension];
  double complex MatrixB[Dimension * Dimension];
  double complex AOriginal[Dimension * Dimension];
  double complex BOriginal[Dimension * Dimension];
  double complex UnitaryU[Dimension * Dimension];
  double complex UnitaryVH[Dimension * Dimension];
  double complex V[Dimension * Dimension];
  double complex EigenvaluesA[Dimension], EigenvaluesB[Dimension];
  double complex AdjointU[Dimension * Dimension];
  double complex Product[Dimension * Dimension];
  double complex Transformed[Dimension * Dimension];
  double complex S[Dimension * Dimension], T[Dimension * Dimension];
  int            Info;
  size_t         i, j;

  MatrixA[0] = 1.0 + 1.0 * I; // (1, 1)
  MatrixA[1] = 0.0;           // (2, 1)
  MatrixA[2] = 2.0 * I;       // (3, 1)
  MatrixA[3] = 2.0;           // (1, 2)
  MatrixA[4] = 3.0 - 1.0 * I; // (2, 2)
  MatrixA[5] = 1.0;           // (3, 2)
  MatrixA[6] = -1.0 * I;      // (1, 3)
  MatrixA[7] = 1.0;           // (2, 3)
  MatrixA[8] = 4.0;           // (3, 3)

  MatrixB[0] = 2.0;           // (1, 1)
  MatrixB[1] = 1.0;           // (2, 1)
  MatrixB[2] = 0.0;           // (3, 1)
  MatrixB[3] = 1.0 * I;       // (1, 2)
  MatrixB[4] = 3.0;           // (2, 2)
  MatrixB[5] = 2.0 * I;       // (3, 2)
  MatrixB[6] = 0.0;           // (1, 3)
  MatrixB[7] = 1.0 - 1.0 * I; // (2, 3)
  MatrixB[8] = 5.0;           // (3, 3)

  for (i = 0; i < Dimension * Dimension; i++) {
    AOriginal[i] = MatrixA[i];
    BOriginal[i] = MatrixB[i];
  }

  GeneralizedSchurDecomposition(Dimension, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, EigenvaluesB, UnitaryVH, zgges_,
                                &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(MatrixA[i] - AOriginal[i]) <= 0.0);
    ASSERT(cabs(MatrixB[i] - BOriginal[i]) <= 0.0);
  }

  // The columns of U and of V are orthonormal.
  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      V[i + j * Dimension]        = conj(UnitaryVH[j + i * Dimension]);
      AdjointU[i + j * Dimension] = conj(UnitaryU[j + i * Dimension]);
    }
  }

  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      if (i == j) {
        ASSERT(fabs(cabs(ColumnDot(Dimension, UnitaryU, i, j)) - 1.0) <
               Tolerance);
        ASSERT(fabs(cabs(ColumnDot(Dimension, V, i, j)) - 1.0) < Tolerance);
      } else {
        ASSERT(cabs(ColumnDot(Dimension, UnitaryU, i, j)) < Tolerance);
        ASSERT(cabs(ColumnDot(Dimension, V, i, j)) < Tolerance);
      }
    }
  }

  MultiplyMatrices(Dimension, AOriginal, V, Product);
  MultiplyMatrices(Dimension, AdjointU, Product, S);
  MultiplyMatrices(Dimension, BOriginal, V, Product);
  MultiplyMatrices(Dimension, AdjointU, Product, T);

  for (j = 0; j < Dimension; j++) {
    for (i = j + 1; i < Dimension; i++) {
      ASSERT(cabs(S[i + j * Dimension]) < Tolerance);
      ASSERT(cabs(T[i + j * Dimension]) < Tolerance);
    }
    ASSERT(fabs(cimag(T[j + j * Dimension])) < Tolerance);
    ASSERT(creal(T[j + j * Dimension]) >= 0.0);
  }

  // A = U*S*V^H and B = U*T*V^H.
  MultiplyMatrices(Dimension, UnitaryU, S, Product);
  MultiplyMatrices(Dimension, Product, UnitaryVH, Transformed);
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Transformed[i] - AOriginal[i]) < Tolerance);
  }

  MultiplyMatrices(Dimension, UnitaryU, T, Product);
  MultiplyMatrices(Dimension, Product, UnitaryVH, Transformed);
  for (i = 0; i < Dimension * Dimension; i++) {
    ASSERT(cabs(Transformed[i] - BOriginal[i]) < Tolerance);
  }

  // The pair ratios agree with the diagonal of the Schur forms.
  for (j = 0; j < Dimension; j++) {
    ASSERT(cabs(EigenvaluesA[j] / EigenvaluesB[j] -
                S[j + j * Dimension] / T[j + j * Dimension]) < Tolerance);
  }
}

// Both eigenvalue outputs are optional; omitting them must not change the
// Schur vectors or the status, and either one may be omitted on its own.
TEST(LDWGeneralizedSchurOptionalEigenvalues) {
  const size_t Dimension = 3;
  const double Tolerance = 1000.0 * DBL_EPSILON;

  double complex MatrixA[Dimension * Dimension];
  double complex MatrixB[Dimension * Dimension];
  double complex UnitaryU[Dimension * Dimension];
  double complex UnitaryVH[Dimension * Dimension];
  double complex EigenvaluesA[Dimension], EigenvaluesB[Dimension];
  double complex V[Dimension * Dimension], AdjointU[Dimension * Dimension];
  double complex Product[Dimension * Dimension];
  double complex S[Dimension * Dimension], T[Dimension * Dimension];
  int            Info;
  size_t         i, j;

  MatrixA[0] = 2.0; // (1, 1)
  MatrixA[1] = 1.0; // (2, 1)
  MatrixA[2] = 4.0; // (3, 1)
  MatrixA[3] = 0.0; // (1, 2)
  MatrixA[4] = 3.0; // (2, 2)
  MatrixA[5] = 5.0; // (3, 2)
  MatrixA[6] = 0.0; // (1, 3)
  MatrixA[7] = 0.0; // (2, 3)
  MatrixA[8] = 4.0; // (3, 3)

  MatrixB[0] = 1.0; // (1, 1)
  MatrixB[1] = 2.0; // (2, 1)
  MatrixB[2] = 1.0; // (3, 1)
  MatrixB[3] = 0.0; // (1, 2)
  MatrixB[4] = 2.0; // (2, 2)
  MatrixB[5] = 1.0; // (3, 2)
  MatrixB[6] = 0.0; // (1, 3)
  MatrixB[7] = 0.0; // (2, 3)
  MatrixB[8] = 3.0; // (3, 3)

  // Both eigenvalue arrays omitted: the Schur vectors are still computed.
  GeneralizedSchurDecomposition(Dimension, MatrixA, MatrixB, UnitaryU, NULL,
                                NULL, UnitaryVH, zgges_, &Info);
  ASSERT(Info == 0);
  if (Info != 0) {
    return;
  }

  for (j = 0; j < Dimension; j++) {
    for (i = 0; i < Dimension; i++) {
      V[i + j * Dimension]        = conj(UnitaryVH[j + i * Dimension]);
      AdjointU[i + j * Dimension] = conj(UnitaryU[j + i * Dimension]);
    }
  }

  MultiplyMatrices(Dimension, MatrixA, V, Product);
  MultiplyMatrices(Dimension, AdjointU, Product, S);
  MultiplyMatrices(Dimension, MatrixB, V, Product);
  MultiplyMatrices(Dimension, AdjointU, Product, T);

  for (j = 0; j < Dimension; j++) {
    for (i = j + 1; i < Dimension; i++) {
      ASSERT(cabs(S[i + j * Dimension]) < Tolerance);
      ASSERT(cabs(T[i + j * Dimension]) < Tolerance);
    }
  }

  // Only the numerators omitted: the denominators are still returned.
  GeneralizedSchurDecomposition(Dimension, MatrixA, MatrixB, UnitaryU, NULL,
                                EigenvaluesB, UnitaryVH, zgges_, &Info);
  ASSERT(Info == 0);
  if (Info == 0) {
    for (j = 0; j < Dimension; j++) {
      ASSERT(cabs(EigenvaluesB[j]) > 0.0);
    }
  }

  // Only the denominators omitted: the numerators are still returned.
  GeneralizedSchurDecomposition(Dimension, MatrixA, MatrixB, UnitaryU,
                                EigenvaluesA, NULL, UnitaryVH, zgges_, &Info);
  ASSERT(Info == 0);
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
           LDWSchurReference,
           LDWSchurCongruence,
           LDWGeneralizedSchurReference,
           LDWGeneralizedSchurCongruence,
           LDWGeneralizedSchurOptionalEigenvalues,
           LDWSVDReference,
           LDWSVDCongruenceTall,
           LDWDegeneracyGroupReference);
