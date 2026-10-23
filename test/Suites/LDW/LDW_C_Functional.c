#include "src/LDW/LapackDeferredWrappers.h"
#include "test/TSD/TSD.h"
#include <complex.h>
#include <float.h>
#include <limits.h>
#include <math.h>

extern void zheevd_(void);
extern void zgees_(void);

void DummyZHEEVD(void) {
  // Dummy.
}

void DummyZGEES(void) {
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
  SchurDecomposition((size_t)0, Matrix, Unitary, NULL, DummyZGEES, &Info);
  ASSERT(Info == 1); // Info unchanged.
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
           LDWDegeneracyGroupReference);
