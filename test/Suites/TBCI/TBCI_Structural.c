#include "src/Core/Core.h"
#include "src/MAC/MultidimensionalArrayContainers.h"
#include "src/TBCI/TightBindingCrystalInterpolation.h"
#include "test/CoreSupport/CoreSupport.h"
#include "test/TSD/TSD.h"
#include <stdint.h>

CONTEXT_FN(StructuralSetup) {
  (void)ContextData;
  Core_Malloc = FallibleMalloc;
  ResetFallibleMalloc();
  Core_MulOverflow = FallibleMulOverflow;
  ResetFallibleMulOverflow();
  Core_AddOverflow = FallibleAddOverflow;
  ResetFallibleAddOverflow();
}
CONTEXT_FN(StructuralTeardown) {
  (void)ContextData;
  RESET_FN(Core_Malloc);
  RESET_FN(Core_MulOverflow);
  RESET_FN(Core_AddOverflow);
}
CONTEXT(StructuralCtx,
        StructuralSetup,
        StructuralTeardown,
        (size_t)0);

TEST(FreeTBCrystal_BadInput) { FreeTBCrystal(NULL); }

TEST(FreeTBCrystal_Nominal) {
  TBCrystal *Crystal = NULL;

  Crystal = NewTBCrystal(1, 1, 1);
  ASSERT(Crystal);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(NewTBCrystal_BadInput) {
  TBCrystal *Crystal = NULL;

  Crystal = NewTBCrystal(0, 1, 1);
  ASSERT(!Crystal);
  Crystal = NewTBCrystal(1, 0, 1);
  ASSERT(!Crystal);
  Crystal = NewTBCrystal(1, 1, 0);
  ASSERT(!Crystal);
}

TEST(NewTBCrystal_Nominal) {
  TBCrystal *Crystal = NULL;

  Crystal = NewTBCrystal(3, 4, 2);
  ASSERT(Crystal);
  ASSERT(TBCrystalGetDimensionality(Crystal) == 3);
  ASSERT(TBCrystalGetSiteCount(Crystal) == 4);
  ASSERT(TBCrystalGetBandCount(Crystal) == 2);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalGetters_Null) {
  ASSERT(TBCrystalGetDimensionality(NULL) == 0);
  ASSERT(TBCrystalGetSiteCount(NULL) == 0);
  ASSERT(TBCrystalGetBandCount(NULL) == 0);
}

TEST_SUITE(TBCIStructuralLifetimeSuite,
           FreeTBCrystal_Nominal,
           NewTBCrystal_Nominal);

TEST_SUITE(TBCIStructuralNullArgumentSuite,
           FreeTBCrystal_BadInput,
           NewTBCrystal_BadInput,
           TBCrystalGetters_Null);

TEST_F(StructuralCtx,
       NewTBCrystal_MallocFail) {
  TBCrystal *Crystal = NULL;
  size_t     FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleMalloc(FailAt);
    Crystal = NewTBCrystal(3, 4, 2);
    if (Crystal) {
      FreeTBCrystal(&Crystal);
      break;
    }
  }
  ASSERT(FailAt == 14);
}

TEST_F(StructuralCtx,
       NewTBCrystal_MulOverflowFail) {
  TBCrystal *Crystal = NULL;
  size_t     FailAt;

  (void)ContextData;
  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleMulOverflow(FailAt);
    Crystal = NewTBCrystal(3, 4, 2);
    if (Crystal) {
      FreeTBCrystal(&Crystal);
      break;
    }
  }
  ASSERT(FailAt == 20);
}

TEST(NewTBCrystal_OverflowInput) {
  ASSERT(!NewTBCrystal(SIZE_MAX, 1, 1));
  ASSERT(!NewTBCrystal(1, SIZE_MAX, 1));
  ASSERT(!NewTBCrystal(1, 2, (size_t)1 << 31));
}

// TBCrystalHamiltonian evaluates the Derivative-th Cartesian k-derivative of
// the Bloch Hamiltonian and specifies the shape of the result on the way, so
// it allocates and it checks arithmetic. The tests below fail each of those
// operations in turn: every failure must come back through the return value,
// and the first fault-free attempt must produce the result. The accessors of
// the stored Hamiltonian share the TBCrystalHamiltonian prefix, hence the
// Bloch qualifier.

TEST_F(StructuralCtx,
       TBCrystalBlochHamiltonian_MallocFail) {
  TBCrystal     *Crystal   = NULL;
  double         KPoint[1] = {0.25};
  double complex Array[1];
  size_t         ArrayShape[2];
  size_t         FailAt;
  bool           Success = false;

  (void)ContextData;

  Crystal = NewTBCrystal(1, 1, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 0, 1.0));

  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleMalloc(FailAt);
    Success =
        TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
    if (Success) {
      break;
    }
  }
  // Four allocations: the specifier record, its shape buffer, the multi-index
  // buffer and the phase buffer.
  ASSERT(FailAt == 5);
  ASSERT(Success);

  // The fault-free attempt evaluated the crystal instead of skipping it.
  ASSERT(ArrayShape[0] == 1);
  ASSERT(ArrayShape[1] == 1);
  ASSERT(creal(Array[0]) == 1.0);

  ASSERT(cimag(Array[0]) == 0.0);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_F(StructuralCtx,
       TBCrystalBlochHamiltonian_MulOverflowFail) {
  TBCrystal     *Crystal   = NULL;
  double         KPoint[1] = {0.25};
  double complex Array[1];
  size_t         ArrayShape[4];
  size_t         Derivative;
  size_t         FailAt;
  size_t         i;
  bool           Success;

  (void)ContextData;

  Crystal = NewTBCrystal(1, 1, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 0, 1.0));

  // One product per shape entry of the (Derivative + 2)-ranked result, plus
  // the byte size of the shape buffer, plus the byte sizes of the multi-index
  // and the phase buffer. The derivatives add the two products of the
  // Cartesian offsets.
  for (Derivative = 0; Derivative <= 2; Derivative++) {
    Success = false;
    for (FailAt = 1; FailAt <= 64; FailAt++) {
      SetFallibleMulOverflow(FailAt);
      Success = TBCrystalHamiltonian(Crystal, 1, KPoint, Derivative, 1, Array,
                                     Derivative + 2, ArrayShape);
      if (Success) {
        break;
      }
    }
    if (Derivative == 0) {
      ASSERT(FailAt == Derivative + 6);
    } else {
      ASSERT(FailAt == Derivative + 8);
    }
    ASSERT(Success);
    ASSERT(ArrayShape[0] == 1);
    ASSERT(ArrayShape[1] == 1);
    for (i = 0; i < Derivative; i++) {
      ASSERT(ArrayShape[2 + i] == 1);
    }
    // The single site sits at the origin, so every derivative of order m > 0
    // vanishes while the m = 0 evaluation returns the on-site energy.
    ASSERT(creal(Array[0]) == (Derivative == 0 ? 1.0 : 0.0));
    ASSERT(cimag(Array[0]) == 0.0);
  }

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_F(StructuralCtx,
       TBCrystalBlochHamiltonian_AddOverflowFail) {
  TBCrystal     *Crystal   = NULL;
  double         KPoint[1] = {0.25};
  double complex Array[1];
  size_t         ArrayShape[2];
  size_t         FailAt;
  bool           Success = false;

  (void)ContextData;

  Crystal = NewTBCrystal(1, 1, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 0, 1.0));

  // A single checked sum on the path, the shape rank Derivative + 2.
  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    Success =
        TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
    if (Success) {
      break;
    }
  }
  ASSERT(FailAt == 2);
  ASSERT(Success);

  // A rank that cannot be represented is rejected, never truncated.
  ResetFallibleAddOverflow();
  ASSERT(!TBCrystalHamiltonian(Crystal, 1, KPoint, SIZE_MAX, 1, Array, 2,
                               ArrayShape));

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

// TBPositionOperator evaluates the Derivative-th Cartesian k-derivative of the
// Bloch Berry connection and specifies the shape of the result on the way, so
// it allocates and it checks arithmetic. The tests below fail each of those
// operations in turn: every failure must come back through the return value,
// and the first fault-free attempt must produce the result. The result carries
// the extra Cartesian component index of the stored connection, so its rank is
// one higher than that of the Hamiltonian kernel. The accessors of the stored
// Berry connection share the TBCrystal prefix, hence the Bloch qualifier.

TEST_F(StructuralCtx,
       TBCrystalBlochPositionOperator_MallocFail) {
  TBCrystal     *Crystal   = NULL;
  double         KPoint[1] = {0.25};
  double complex Array[1];
  size_t         ArrayShape[3];
  size_t         FailAt;
  bool           Success = false;

  (void)ContextData;

  Crystal = NewTBCrystal(1, 1, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 0, 1.0));

  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleMalloc(FailAt);
    Success =
        TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
    if (Success) {
      break;
    }
  }
  // Four allocations: the specifier record, its shape buffer, the multi-index
  // buffer and the phase buffer.
  ASSERT(FailAt == 5);
  ASSERT(Success);

  // The fault-free attempt evaluated the crystal instead of skipping it.
  ASSERT(ArrayShape[0] == 1);
  ASSERT(ArrayShape[1] == 1);
  ASSERT(ArrayShape[2] == 1);
  ASSERT(creal(Array[0]) == 1.0);
  ASSERT(cimag(Array[0]) == 0.0);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_F(StructuralCtx,
       TBCrystalBlochPositionOperator_MulOverflowFail) {
  TBCrystal     *Crystal   = NULL;
  double         KPoint[1] = {0.25};
  double complex Array[1];
  size_t         ArrayShape[5];
  size_t         Derivative;
  size_t         FailAt;
  size_t         i;
  bool           Success;

  (void)ContextData;

  Crystal = NewTBCrystal(1, 1, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 0, 1.0));

  // One product per shape entry of the (Derivative + 3)-ranked result, plus
  // the byte size of the shape buffer, plus the byte sizes of the multi-index
  // and the phase buffer. The derivatives add the two products of the
  // Cartesian offsets.
  for (Derivative = 0; Derivative <= 2; Derivative++) {
    Success = false;
    for (FailAt = 1; FailAt <= 64; FailAt++) {
      SetFallibleMulOverflow(FailAt);
      Success = TBPositionOperator(Crystal, 1, KPoint, Derivative, 1, Array,
                                   Derivative + 3, ArrayShape);
      if (Success) {
        break;
      }
    }
    if (Derivative == 0) {
      ASSERT(FailAt == Derivative + 7);
    } else {
      ASSERT(FailAt == Derivative + 9);
    }
    ASSERT(Success);
    ASSERT(ArrayShape[0] == 1);
    ASSERT(ArrayShape[1] == 1);
    ASSERT(ArrayShape[2] == 1);
    for (i = 0; i < Derivative; i++) {
      ASSERT(ArrayShape[3 + i] == 1);
    }
    // The single site sits at the origin, so every derivative of order m > 0
    // vanishes while the m = 0 evaluation returns the stored connection.
    ASSERT(creal(Array[0]) == (Derivative == 0 ? 1.0 : 0.0));
    ASSERT(cimag(Array[0]) == 0.0);
  }

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_F(StructuralCtx,
       TBCrystalBlochPositionOperator_AddOverflowFail) {
  TBCrystal     *Crystal   = NULL;
  double         KPoint[1] = {0.25};
  double complex Array[1];
  size_t         ArrayShape[3];
  size_t         FailAt;
  bool           Success = false;

  (void)ContextData;

  Crystal = NewTBCrystal(1, 1, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 0, 1.0));

  // A single checked sum on the path, the shape rank Derivative + 3.
  for (FailAt = 1; FailAt <= 64; FailAt++) {
    SetFallibleAddOverflow(FailAt);
    Success =
        TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
    if (Success) {
      break;
    }
  }
  ASSERT(FailAt == 2);
  ASSERT(Success);

  // A rank that cannot be represented is rejected, never truncated.
  ResetFallibleAddOverflow();
  ASSERT(!TBPositionOperator(Crystal, 1, KPoint, SIZE_MAX, 1, Array, 3,
                             ArrayShape));

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_SUITE(TBCIStructuralAllocationSuite,
           NewTBCrystal_MallocFail,
           NewTBCrystal_MulOverflowFail,
           NewTBCrystal_OverflowInput,
           TBCrystalBlochHamiltonian_MallocFail,
           TBCrystalBlochHamiltonian_MulOverflowFail,
           TBCrystalBlochHamiltonian_AddOverflowFail,
           TBCrystalBlochPositionOperator_MallocFail,
           TBCrystalBlochPositionOperator_MulOverflowFail,
           TBCrystalBlochPositionOperator_AddOverflowFail);

TEST(TBCrystalDirectLattice_BadInput) {
  TBCrystal *Crystal = NULL;
  double     Val     = 0x1234;

  Crystal = NewTBCrystal(2, 1, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, 1.0));
  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 1, 1, 2.0));

  ASSERT(!TBCrystalSetDirectLatticeVal(NULL, 0, 0, 5.0));
  ASSERT(!TBCrystalSetDirectLatticeVal(Crystal, 2, 0, 5.0));
  ASSERT(!TBCrystalSetDirectLatticeVal(Crystal, 0, 2, 5.0));
  ASSERT(!TBCrystalSetDirectLatticeVal(Crystal, SIZE_MAX, 0, 5.0));
  ASSERT(!TBCrystalSetDirectLatticeVal(Crystal, 0, SIZE_MAX, 5.0));

  ASSERT(!TBCrystalGetDirectLatticeVal(NULL, 0, 0, &Val));
  ASSERT(Val == 0x1234);
  ASSERT(!TBCrystalGetDirectLatticeVal(Crystal, 2, 0, &Val));
  ASSERT(Val == 0x1234);
  ASSERT(!TBCrystalGetDirectLatticeVal(Crystal, 0, 2, &Val));
  ASSERT(Val == 0x1234);
  ASSERT(!TBCrystalGetDirectLatticeVal(Crystal, 0, 0, NULL));

  Val = 0.0;
  ASSERT(TBCrystalGetDirectLatticeVal(Crystal, 0, 0, &Val));
  ASSERT(Val == 1.0);
  ASSERT(TBCrystalGetDirectLatticeVal(Crystal, 1, 1, &Val));
  ASSERT(Val == 2.0);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalSites_BadInput) {
  TBCrystal *Crystal = NULL;
  long int   Val     = 0x12345678;

  Crystal = NewTBCrystal(2, 3, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetSiteVal(Crystal, 0, 0, 11));
  ASSERT(TBCrystalSetSiteVal(Crystal, 2, 1, 22));

  ASSERT(!TBCrystalSetSiteVal(NULL, 0, 0, 5));
  ASSERT(!TBCrystalSetSiteVal(Crystal, 3, 0, 5));
  ASSERT(!TBCrystalSetSiteVal(Crystal, 0, 2, 5));
  ASSERT(!TBCrystalSetSiteVal(Crystal, SIZE_MAX, 0, 5));
  ASSERT(!TBCrystalSetSiteVal(Crystal, 0, SIZE_MAX, 5));

  ASSERT(!TBCrystalGetSiteVal(NULL, 0, 0, &Val));
  ASSERT(Val == 0x12345678);
  ASSERT(!TBCrystalGetSiteVal(Crystal, 3, 0, &Val));
  ASSERT(Val == 0x12345678);
  ASSERT(!TBCrystalGetSiteVal(Crystal, 0, 2, &Val));
  ASSERT(Val == 0x12345678);
  ASSERT(!TBCrystalGetSiteVal(Crystal, 0, 0, NULL));

  Val = 0;
  ASSERT(TBCrystalGetSiteVal(Crystal, 0, 0, &Val));
  ASSERT(Val == 11);
  ASSERT(TBCrystalGetSiteVal(Crystal, 2, 1, &Val));
  ASSERT(Val == 22);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalHamiltonian_BadInput) {
  TBCrystal     *Crystal = NULL;
  double complex Val     = 0x1234 + 0x5678 * I;

  Crystal = NewTBCrystal(2, 3, 2);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 0, 1.0 + 2.0 * I));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 1, 1, 2, 3.0 + 4.0 * I));

  ASSERT(!TBCrystalSetTBHamiltonian(NULL, 0, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBHamiltonian(Crystal, 2, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBHamiltonian(Crystal, 0, 2, 0, 5.0));
  ASSERT(!TBCrystalSetTBHamiltonian(Crystal, 0, 0, 3, 5.0));
  ASSERT(!TBCrystalSetTBHamiltonian(Crystal, SIZE_MAX, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBHamiltonian(Crystal, 0, SIZE_MAX, 0, 5.0));
  ASSERT(!TBCrystalSetTBHamiltonian(Crystal, 0, 0, SIZE_MAX, 5.0));

  ASSERT(!TBCrystalGetTBHamiltonian(NULL, 0, 0, 0, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(cimag(Val) == 0x5678);
  ASSERT(!TBCrystalGetTBHamiltonian(Crystal, 2, 0, 0, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(!TBCrystalGetTBHamiltonian(Crystal, 0, 2, 0, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(!TBCrystalGetTBHamiltonian(Crystal, 0, 0, 3, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(!TBCrystalGetTBHamiltonian(Crystal, 0, 0, 0, NULL));

  Val = 0.0;
  ASSERT(TBCrystalGetTBHamiltonian(Crystal, 0, 0, 0, &Val));
  ASSERT(creal(Val) == 1.0);
  ASSERT(cimag(Val) == 2.0);
  ASSERT(TBCrystalGetTBHamiltonian(Crystal, 1, 1, 2, &Val));
  ASSERT(creal(Val) == 3.0);
  ASSERT(cimag(Val) == 4.0);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalPositionOperator_BadInput) {
  TBCrystal     *Crystal = NULL;
  double complex Val     = 0x1234 + 0x5678 * I;

  Crystal = NewTBCrystal(2, 2, 2);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 0, 1.0 + 2.0 * I));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 1, 1, 1, 1, 3.0 + 4.0 * I));

  ASSERT(!TBCrystalSetTBPositionOperator(NULL, 0, 0, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, 2, 0, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, 0, 2, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, 0, 0, 2, 0, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 2, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, SIZE_MAX, 0, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, 0, SIZE_MAX, 0, 0, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, 0, 0, SIZE_MAX, 0, 5.0));
  ASSERT(!TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, SIZE_MAX, 5.0));

  ASSERT(!TBCrystalGetTBPositionOperator(NULL, 0, 0, 0, 0, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(cimag(Val) == 0x5678);
  ASSERT(!TBCrystalGetTBPositionOperator(Crystal, 2, 0, 0, 0, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(!TBCrystalGetTBPositionOperator(Crystal, 0, 2, 0, 0, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(!TBCrystalGetTBPositionOperator(Crystal, 0, 0, 2, 0, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(!TBCrystalGetTBPositionOperator(Crystal, 0, 0, 0, 2, &Val));
  ASSERT(creal(Val) == 0x1234);
  ASSERT(!TBCrystalGetTBPositionOperator(Crystal, 0, 0, 0, 0, NULL));

  Val = 0.0;
  ASSERT(TBCrystalGetTBPositionOperator(Crystal, 0, 0, 0, 0, &Val));
  ASSERT(creal(Val) == 1.0);
  ASSERT(cimag(Val) == 2.0);
  ASSERT(TBCrystalGetTBPositionOperator(Crystal, 1, 1, 1, 1, &Val));
  ASSERT(creal(Val) == 3.0);
  ASSERT(cimag(Val) == 4.0);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

typedef struct TBCrystal {
  MAC            *DirectLatticeSpec;
  double         *DirectLattice;
  MAC            *SiteCoordsSpec;
  long int       *SiteCoords;
  MAC            *HamiltonianSpec;
  double complex *Hamiltonian;
  MAC            *PositionOpSpec;
  double complex *PositionOp;
  size_t          Dimensionality;
  size_t          SiteCount;
  size_t          BandCount;
} TBCrystalCopy;

TEST(TBCIStructuralDefectiveObjectTest) {
  TBCrystal     *Crystal    = NULL;
  TBCrystalCopy *CrystalPtr = NULL;
  void          *Tmp;

  double         KPoint[1] = {0.25};
  double complex Array[1];
  size_t         ArrayShape[3];

  Crystal    = NewTBCrystal(1, 1, 1);
  CrystalPtr = (TBCrystalCopy *)Crystal;

  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 0, 1.0));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 0, 2.0));

  // Corrupt field: DirectLattice.
  Tmp                       = CrystalPtr->DirectLattice;
  CrystalPtr->DirectLattice = NULL;
  ASSERT(!TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape));
  CrystalPtr->DirectLattice = Tmp;

  // Corrupt field: SiteCoords.
  Tmp                    = CrystalPtr->SiteCoords;
  CrystalPtr->SiteCoords = NULL;
  ASSERT(!TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape));
  CrystalPtr->SiteCoords = Tmp;

  // Corrupt field: Hamiltonian.
  Tmp                     = CrystalPtr->Hamiltonian;
  CrystalPtr->Hamiltonian = NULL;
  ASSERT(!TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape));
  CrystalPtr->Hamiltonian = Tmp;

  // Corrupt field: PositionOp.
  Tmp                    = CrystalPtr->PositionOp;
  CrystalPtr->PositionOp = NULL;
  ASSERT(!TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape));
  CrystalPtr->PositionOp = Tmp;

  // The repaired object evaluates both kernels again.
  ASSERT(TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape));
  ASSERT(creal(Array[0]) == 1.0);
  ASSERT(TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape));
  ASSERT(creal(Array[0]) == 2.0);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_SUITE(TBCIStructuralFunctionReturnSuite,
           TBCrystalDirectLattice_BadInput,
           TBCrystalSites_BadInput,
           TBCrystalHamiltonian_BadInput,
           TBCrystalPositionOperator_BadInput,
           TBCIStructuralDefectiveObjectTest);
