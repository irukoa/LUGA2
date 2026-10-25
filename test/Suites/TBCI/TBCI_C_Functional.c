#include "src/TBCI/TightBindingCrystalInterpolation.h"
#include "test/TSD/TSD.h"
#include <math.h>
#include <stdint.h>

TEST(TBCrystalCounts_Nominal) {
  TBCrystal *Crystal = NULL;

  Crystal = NewTBCrystal(3, 4, 2);
  ASSERT(Crystal);
  ASSERT(TBCrystalGetDimensionality(Crystal) == 3);
  ASSERT(TBCrystalGetSiteCount(Crystal) == 4);
  ASSERT(TBCrystalGetBandCount(Crystal) == 2);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_SUITE(TBCIFunctionalUtilitySuite,
           TBCrystalCounts_Nominal);

TEST(TBCrystalDirectLattice_Nominal) {
  TBCrystal *Crystal = NULL;
  double     Val;
  size_t     VectorId;
  size_t     ComponentId;

  Crystal = NewTBCrystal(3, 1, 1);
  ASSERT(Crystal);

  for (VectorId = 0; VectorId < 3; VectorId++) {
    for (ComponentId = 0; ComponentId < 3; ComponentId++) {
      ASSERT(TBCrystalSetDirectLatticeVal(Crystal, VectorId, ComponentId,
                                          100.0 * (double)VectorId +
                                              (double)ComponentId));
    }
  }
  for (VectorId = 0; VectorId < 3; VectorId++) {
    for (ComponentId = 0; ComponentId < 3; ComponentId++) {
      Val = -1.0;
      ASSERT(
          TBCrystalGetDirectLatticeVal(Crystal, VectorId, ComponentId, &Val));
      ASSERT(Val == 100.0 * (double)VectorId + (double)ComponentId);
    }
  }

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalSites_Nominal) {
  TBCrystal *Crystal = NULL;
  long int   Val;
  size_t     SiteId;
  size_t     ComponentId;

  Crystal = NewTBCrystal(3, 4, 2);
  ASSERT(Crystal);

  for (SiteId = 0; SiteId < 4; SiteId++) {
    for (ComponentId = 0; ComponentId < 3; ComponentId++) {
      ASSERT(TBCrystalSetSiteVal(Crystal, SiteId, ComponentId,
                                 (long int)(1000 * SiteId + ComponentId) - 7));
    }
  }
  for (SiteId = 0; SiteId < 4; SiteId++) {
    for (ComponentId = 0; ComponentId < 3; ComponentId++) {
      Val = 0;
      ASSERT(TBCrystalGetSiteVal(Crystal, SiteId, ComponentId, &Val));
      ASSERT(Val == (long int)(1000 * SiteId + ComponentId) - 7);
    }
  }

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalHamiltonian_Nominal) {
  TBCrystal     *Crystal = NULL;
  double complex Val;
  size_t         BandId1;
  size_t         BandId2;
  size_t         SiteId;

  Crystal = NewTBCrystal(2, 2, 3);
  ASSERT(Crystal);

  for (BandId1 = 0; BandId1 < 3; BandId1++) {
    for (BandId2 = 0; BandId2 < 3; BandId2++) {
      for (SiteId = 0; SiteId < 2; SiteId++) {
        ASSERT(TBCrystalSetTBHamiltonian(
            Crystal, BandId1, BandId2, SiteId,
            (double)(BandId1 + 10 * BandId2 + 100 * SiteId) +
                (1000.0 + (double)(BandId1 + 10 * BandId2 + 100 * SiteId)) *
                    I));
      }
    }
  }
  for (BandId1 = 0; BandId1 < 3; BandId1++) {
    for (BandId2 = 0; BandId2 < 3; BandId2++) {
      for (SiteId = 0; SiteId < 2; SiteId++) {
        Val = 0.0;
        ASSERT(
            TBCrystalGetTBHamiltonian(Crystal, BandId1, BandId2, SiteId, &Val));
        ASSERT(creal(Val) == (double)(BandId1 + 10 * BandId2 + 100 * SiteId));
        ASSERT(cimag(Val) ==
               1000.0 + (double)(BandId1 + 10 * BandId2 + 100 * SiteId));
      }
    }
  }

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalPositionOperator_Nominal) {
  TBCrystal     *Crystal = NULL;
  double complex Val;
  size_t         BandId1;
  size_t         BandId2;
  size_t         SiteId;
  size_t         ComponentId;

  Crystal = NewTBCrystal(3, 2, 2);
  ASSERT(Crystal);

  for (BandId1 = 0; BandId1 < 2; BandId1++) {
    for (BandId2 = 0; BandId2 < 2; BandId2++) {
      for (SiteId = 0; SiteId < 2; SiteId++) {
        for (ComponentId = 0; ComponentId < 3; ComponentId++) {
          ASSERT(TBCrystalSetTBPositionOperator(
              Crystal, BandId1, BandId2, SiteId, ComponentId,
              (double)(BandId1 + 10 * BandId2 + 100 * SiteId +
                       1000 * ComponentId) +
                  (10000.0 + (double)(BandId1 + 10 * BandId2 + 100 * SiteId +
                                      1000 * ComponentId)) *
                      I));
        }
      }
    }
  }
  for (BandId1 = 0; BandId1 < 2; BandId1++) {
    for (BandId2 = 0; BandId2 < 2; BandId2++) {
      for (SiteId = 0; SiteId < 2; SiteId++) {
        for (ComponentId = 0; ComponentId < 3; ComponentId++) {
          Val = 0.0;
          ASSERT(TBCrystalGetTBPositionOperator(Crystal, BandId1, BandId2,
                                                SiteId, ComponentId, &Val));
          ASSERT(creal(Val) == (double)(BandId1 + 10 * BandId2 + 100 * SiteId +
                                        1000 * ComponentId));
          ASSERT(cimag(Val) ==
                 10000.0 + (double)(BandId1 + 10 * BandId2 + 100 * SiteId +
                                    1000 * ComponentId));
        }
      }
    }
  }

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_SUITE(TBCIFunctionalAccessorSuite,
           TBCrystalDirectLattice_Nominal,
           TBCrystalSites_Nominal,
           TBCrystalHamiltonian_Nominal,
           TBCrystalPositionOperator_Nominal);

// TBCrystalHamiltonian evaluates the Derivative-th Cartesian k-derivative of
// the Bloch Hamiltonian,
//
//     d^m H(k)/dk^m = d^m/dk^m sum_S H(b1, b2, S) exp(i k.R_S),
//
// and reports the shape of the result. The accessors of the stored
// Hamiltonian share the TBCrystalHamiltonian prefix, hence the Bloch
// qualifier.

TEST(TBCrystalBlochHamiltonian_ChainDispersion) {
  TBCrystal     *Crystal          = NULL;
  const double   Pi               = acos(-1.0);
  const double   Tolerance        = 1.0e-12;
  const double   Hop              = -1.25;
  const double   Onsite           = 0.5;
  const double   Lattice          = 2.0;
  const double   ReducedPoints[7] = {0.0, 0.125, 0.25, 1.0 / 3.0,
                                     0.5, -0.25, -0.5};
  double         KPoint[1];
  double complex Array[1];
  double complex Reference;
  size_t         ArrayShape[2];
  size_t         i;
  bool           Success;

  // A one-dimensional, one-band chain with nearest-neighbour hopping t
  // between the sites at R = -a, R = 0 and R = +a,
  //
  //     ... -- o -- o -- o -- ...
  //            R=-a  R=0  R=+a
  //
  // so that
  //
  //     H(k) = e + t exp(-i k a) + t exp(+i k a) = e + 2 t cos(2 pi x),
  //
  // with k = 2 pi x / a and x = KPoint[0]: a sine band of offset e and
  // amplitude 2 |t|, periodic in x with period 1.

  Crystal = NewTBCrystal(1, 3, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, Lattice));
  ASSERT(TBCrystalSetSiteVal(Crystal, 0, 0, -1));
  ASSERT(TBCrystalSetSiteVal(Crystal, 1, 0, 0));
  ASSERT(TBCrystalSetSiteVal(Crystal, 2, 0, 1));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 0, Hop));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 1, Onsite));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 2, Hop));

  // The analytic dispersion is recovered across the reduced zone.
  for (i = 0; i < 7; i++) {
    KPoint[0]     = ReducedPoints[i];
    ArrayShape[0] = 0;
    ArrayShape[1] = 0;
    Success =
        TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
    ASSERT_X(Success);
    ASSERT(ArrayShape[0] == 1);
    ASSERT(ArrayShape[1] == 1);
    Reference = Onsite + 2.0 * Hop * cos(2.0 * Pi * ReducedPoints[i]);
    ASSERT(cabs(Array[0] - Reference) < Tolerance);
    ASSERT(fabs(cimag(Array[0])) < Tolerance);
  }

  // Band bottom at x = 0, band top at x = 1/2 and the crossing at e for
  // x = 1/4.
  KPoint[0] = 0.0;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - (Onsite + 2.0 * Hop)) < Tolerance);

  KPoint[0] = 0.5;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - (Onsite - 2.0 * Hop)) < Tolerance);

  KPoint[0] = 0.25;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Onsite) < Tolerance);

  // cos(2 pi x) = sin(2 pi (x + 1/4)): the band is a plain sine wave.
  KPoint[0] = 0.1;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  Reference = Onsite + 2.0 * Hop * sin(2.0 * Pi * 0.35);
  ASSERT(cabs(Array[0] - Reference) < Tolerance);

  // The phase is periodic in every reduced coordinate with period 1, so
  // replicas of the same point must agree.
  KPoint[0] = 0.2;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  Reference = Array[0];

  KPoint[0] = 1.2;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Reference) < Tolerance);

  KPoint[0] = -1.8;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Reference) < Tolerance);

  // x is a reduced coordinate, so the zeroth derivative does not depend on
  // the length of the direct lattice vector: only k a enters the phase.
  KPoint[0] = 0.3;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  Reference = Array[0];

  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, 3.7));
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 1, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Reference) < Tolerance);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalBlochHamiltonian_ChainDerivatives) {
  TBCrystal     *Crystal          = NULL;
  const double   Pi               = acos(-1.0);
  const double   Tolerance        = 1.0e-12;
  const double   Hop              = -1.25;
  const double   Onsite           = 0.5;
  const double   Lattice          = 2.0;
  const double   ReducedPoints[5] = {0.0, 0.125, 0.25, 0.5, -0.375};
  double         KPoint[1];
  double complex Array[1];
  double complex Reference;
  size_t         ArrayShape[6];
  size_t         i;
  size_t         m;
  size_t         t;
  bool           Success;

  // The chain of the dispersion benchmark has H(k) = e + 2 t cos(k a) with
  // k = 2 pi x / a, hence
  //
  //     d^m H / dk^m = 2 t a^m cos(2 pi x + m pi / 2),
  //
  // which pins the chain rule (powers of a), the i^m factor and the
  //(m + 2)-ranked shape of the result at once.

  Crystal = NewTBCrystal(1, 3, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, Lattice));
  ASSERT(TBCrystalSetSiteVal(Crystal, 0, 0, -1));
  ASSERT(TBCrystalSetSiteVal(Crystal, 1, 0, 0));
  ASSERT(TBCrystalSetSiteVal(Crystal, 2, 0, 1));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 0, Hop));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 1, Onsite));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 2, Hop));

  for (m = 1; m <= 4; m++) {
    for (i = 0; i < 5; i++) {
      KPoint[0] = ReducedPoints[i];
      for (t = 0; t < m + 2; t++) {
        ArrayShape[t] = 0;
      }
      Success = TBCrystalHamiltonian(Crystal, 1, KPoint, m, 1, Array, m + 2,
                                     ArrayShape);
      ASSERT_X(Success);

      // BandCount, BandCount and then one entry per derivative direction.
      ASSERT(ArrayShape[0] == 1);
      ASSERT(ArrayShape[1] == 1);
      for (t = 0; t < m; t++) {
        ASSERT(ArrayShape[2 + t] == 1);
      }

      Reference = 2.0 * Hop * pow(Lattice, (double)m) *
                  cos(2.0 * Pi * ReducedPoints[i] + 0.5 * (double)m * Pi);
      ASSERT(cabs(Array[0] - Reference) < Tolerance);
      ASSERT(fabs(cimag(Array[0])) < Tolerance);
    }
  }

  // At a band edge the odd derivatives vanish and the even ones reduce to
  //-2 t a^m at x = 0 and +2 t a^m at x = 1/2.
  KPoint[0] = 0.0;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 1, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0]) < Tolerance);

  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 2, 1, Array, 4, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - (-2.0 * Hop * Lattice * Lattice)) < Tolerance);

  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 3, 1, Array, 5, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0]) < Tolerance);

  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 4, 1, Array, 6, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - 2.0 * Hop * pow(Lattice, 4.0)) < Tolerance);

  KPoint[0] = 0.5;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 2, 1, Array, 4, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - 2.0 * Hop * Lattice * Lattice) < Tolerance);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalBlochHamiltonian_Layout) {
  TBCrystal   *Crystal   = NULL;
  TBCrystal   *Skewed    = NULL;
  const double Pi        = acos(-1.0);
  const double Tolerance = 1.0e-12;
  const size_t Dim       = 2;
  const size_t Bands     = 2;
  const size_t Sites     = 4;
  const size_t MaxRank   = 4;
  // Lattice[ComponentId][VectorId]: column j holds the vector a_j.
  const double   Lattice[2][2] = {{1.0, 0.5}, {0.0, 2.0}};
  const long int Coords[2][4]  = {{0, 1, 0, 1}, {0, 0, 1, 1}};
  const double   Origin[2]     = {0.0, 0.0};
  double         KPoint[2]     = {0.125, -0.375};
  double complex Blocks[2][2][4];
  double complex Array[16];
  double complex SkewedArray[4];
  double complex Reference;
  double complex Sum;
  double complex Ipow;
  double complex Expected;
  size_t         ArrayShape[4];
  size_t         SkewedShape[2];
  size_t         Direction[2];
  size_t         b1;
  size_t         b2;
  size_t         s;
  size_t         c;
  size_t         m;
  size_t         t;
  size_t         i;
  size_t         idx;
  size_t         Elements;
  double         KdotR;
  double         DerProd;
  double         Pos[2];
  bool           Success;

  // Two dimensions, two bands and four sites on a non-orthogonal lattice, so
  // that the Cartesian position of site s is
  //
  //     R = a1 Coords[0][s] + a2 Coords[1][s],
  //
  // with a1 = (1, 0) and a2 = (1/2, 2): neither symmetric nor orthogonal, so
  // a transposed read of the direct lattice cannot go unnoticed. The blocks
  // are deliberately non-Hermitian and asymmetric under BandId1 <-> BandId2,
  // which exposes any transposition, hermitization or direction mix-up.

  Crystal = NewTBCrystal(Dim, Sites, Bands);
  ASSERT(Crystal);

  for (c = 0; c < Dim; c++) {
    for (s = 0; s < Dim; s++) {
      ASSERT(TBCrystalSetDirectLatticeVal(Crystal, s, c, Lattice[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (c = 0; c < Dim; c++) {
      ASSERT(TBCrystalSetSiteVal(Crystal, s, c, Coords[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (b2 = 0; b2 < Bands; b2++) {
      for (b1 = 0; b1 < Bands; b1++) {
        Reference         = (double)(b1 + 10 * b2 + 100 * s) +
                            (1000.0 + (double)(b1 + 10 * b2 + 100 * s)) * I;
        Blocks[b1][b2][s] = Reference;
        ASSERT(TBCrystalSetTBHamiltonian(Crystal, b1, b2, s, Reference));
      }
    }
  }

  for (m = 0; m <= 2; m++) {
    Elements = Bands * Bands;
    for (t = 0; t < m; t++) {
      Elements *= Dim;
    }

    // Only the reported extents are written; the rest of the buffer is left
    // untouched.
    for (t = 0; t < MaxRank; t++) {
      ArrayShape[t] = (size_t)-1;
    }

    Success = TBCrystalHamiltonian(Crystal, Dim, KPoint, m, Elements, Array,
                                   m + 2, ArrayShape);
    ASSERT_X(Success);
    ASSERT(ArrayShape[0] == Bands);
    ASSERT(ArrayShape[1] == Bands);
    for (t = 0; t < m; t++) {
      ASSERT(ArrayShape[2 + t] == Dim);
    }
    for (t = m + 2; t < MaxRank; t++) {
      ASSERT(ArrayShape[t] == (size_t)-1);
    }

    switch (m % 4) {
    case 0:
      Ipow = 1.0;
      break;
    case 1:
      Ipow = I;
      break;
    default:
      Ipow = -1.0;
      break;
    }

    for (i = 0; i < Elements; i++) {
      // Column-major: BandId1 is the fastest index, then BandId2, then the
      // direction of each successive derivative.
      idx = i;
      b1  = idx % Bands;
      idx = idx / Bands;
      b2  = idx % Bands;
      idx = idx / Bands;
      for (t = 0; t < m; t++) {
        Direction[t] = idx % Dim;
        idx          = idx / Dim;
      }

      Sum = 0.0;
      for (s = 0; s < Sites; s++) {
        KdotR   = 2.0 * Pi *
                  ((double)Coords[0][s] * KPoint[0] +
                   (double)Coords[1][s] * KPoint[1]);
        Pos[0]  = Lattice[0][0] * (double)Coords[0][s] +
                  Lattice[0][1] * (double)Coords[1][s];
        Pos[1]  = Lattice[1][0] * (double)Coords[0][s] +
                  Lattice[1][1] * (double)Coords[1][s];
        DerProd = 1.0;
        for (t = 0; t < m; t++) {
          DerProd *= Pos[Direction[t]];
        }
        Sum += Ipow * DerProd * cexp(I * KdotR) * Blocks[b1][b2][s];
      }
      ASSERT(cabs(Array[i] - Sum) < Tolerance);
    }
  }

  //(BandId1, BandId2) = (0, 1) and (1, 0) are returned as stored: no
  // hermitian conjugation, and BandId1 is the fastest running index.
  Skewed   = NewTBCrystal(Dim, 1, Bands);
  Expected = 1.0 + 1.0 * I;
  ASSERT(Skewed);
  ASSERT(TBCrystalSetTBHamiltonian(Skewed, 0, 1, 0, Expected));

  Success = TBCrystalHamiltonian(Skewed, Dim, Origin, 0, 4, SkewedArray, 2,
                                 SkewedShape);
  ASSERT_X(Success);
  ASSERT(SkewedShape[0] == Bands);
  ASSERT(SkewedShape[1] == Bands);
  // Element 2 is (BandId1, BandId2) = (0, 1), element 1 is (1, 0).
  ASSERT(cabs(SkewedArray[2] - Expected) < Tolerance);
  ASSERT(cabs(SkewedArray[0]) < Tolerance);
  ASSERT(cabs(SkewedArray[1]) < Tolerance);
  ASSERT(cabs(SkewedArray[3]) < Tolerance);

  FreeTBCrystal(&Skewed);
  ASSERT(!Skewed);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

// Every other layout test of this module uses two bands and two dimensions,
// where the band stride and the component stride are both 2. There a
// hard-coded 2, a band stride taken from the dimensionality or a component
// stride taken from the band count still lands on the same flat index. This
// test repeats the layout check with three bands, three dimensions and five
// sites, so the strides 3 (bands), 9 (second band) and 3 (component) cannot
// cancel any more.

TEST(TBCrystalBlochHamiltonian_StridedLayout) {
  TBCrystal   *Crystal   = NULL;
  TBCrystal   *Probe     = NULL;
  const double Pi        = acos(-1.0);
  const double Tolerance = 1.0e-12;
  const size_t Dim       = 3;
  const size_t Bands     = 3;
  const size_t Sites     = 5;
  const size_t MaxRank   = 5;
  // Lattice[ComponentId][VectorId]: column j holds the vector a_j. The three
  // vectors are neither orthogonal nor symmetric.
  const double   Lattice[3][3] = {{1.0, 0.25, -0.5},
                                  {0.0, 1.5, 0.75},
                                  {0.0, 0.0, 2.0}};
  const long int Coords[3][5]  = {{0, 1, 0, 0, 1},
                                  {0, 0, 1, 0, -1},
                                  {0, 0, 0, 1, 1}};
  const double   Origin[3]     = {0.0, 0.0, 0.0};
  double         KPoint[3]     = {0.125, 0.3, -0.4};
  double complex Blocks[3][3][5];
  double complex Array[81];
  double complex ProbeArray[9];
  double complex Reference;
  double complex Sum;
  double complex Ipow;
  size_t         ArrayShape[5];
  size_t         ProbeShape[2];
  size_t         Direction[2];
  size_t         b1;
  size_t         b2;
  size_t         s;
  size_t         c;
  size_t         m;
  size_t         t;
  size_t         v;
  size_t         i;
  size_t         idx;
  size_t         Elements;
  double         KdotR;
  double         DerProd;
  double         Pos[3];
  bool           Success;

  Crystal = NewTBCrystal(Dim, Sites, Bands);
  ASSERT(Crystal);

  for (c = 0; c < Dim; c++) {
    for (s = 0; s < Dim; s++) {
      ASSERT(TBCrystalSetDirectLatticeVal(Crystal, s, c, Lattice[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (c = 0; c < Dim; c++) {
      ASSERT(TBCrystalSetSiteVal(Crystal, s, c, Coords[c][s]));
    }
  }

  // Every block is distinct in both parts and asymmetric under
  // BandId1 <-> BandId2, so a swapped, dropped or hermitized read changes the
  // result whatever the strides are.
  for (s = 0; s < Sites; s++) {
    for (b2 = 0; b2 < Bands; b2++) {
      for (b1 = 0; b1 < Bands; b1++) {
        Reference         = (double)(1 + b1 + 3 * b2 + 10 * s) +
                            (-(double)(5 + 2 * b1 + 4 * b2 + 7 * s) - 0.25) * I;
        Blocks[b1][b2][s] = Reference;
        ASSERT(TBCrystalSetTBHamiltonian(Crystal, b1, b2, s, Reference));
      }
    }
  }

  for (m = 0; m <= 2; m++) {
    Elements = Bands * Bands;
    for (t = 0; t < m; t++) {
      Elements *= Dim;
    }

    for (t = 0; t < MaxRank; t++) {
      ArrayShape[t] = (size_t)-1;
    }

    Success = TBCrystalHamiltonian(Crystal, Dim, KPoint, m, Elements, Array,
                                   m + 2, ArrayShape);
    ASSERT_X(Success);
    ASSERT(ArrayShape[0] == Bands);
    ASSERT(ArrayShape[1] == Bands);
    for (t = 0; t < m; t++) {
      ASSERT(ArrayShape[2 + t] == Dim);
    }
    for (t = m + 2; t < MaxRank; t++) {
      ASSERT(ArrayShape[t] == (size_t)-1);
    }

    switch (m % 4) {
    case 0:
      Ipow = 1.0;
      break;
    case 1:
      Ipow = I;
      break;
    default:
      Ipow = -1.0;
      break;
    }

    for (i = 0; i < Elements; i++) {
      // Column-major: BandId1 is the fastest index, then BandId2, then the
      // direction of each successive derivative.
      idx = i;
      b1  = idx % Bands;
      idx = idx / Bands;
      b2  = idx % Bands;
      idx = idx / Bands;
      for (t = 0; t < m; t++) {
        Direction[t] = idx % Dim;
        idx          = idx / Dim;
      }

      Sum = 0.0;
      for (s = 0; s < Sites; s++) {
        KdotR = 2.0 * Pi *
                ((double)Coords[0][s] * KPoint[0] +
                 (double)Coords[1][s] * KPoint[1] +
                 (double)Coords[2][s] * KPoint[2]);
        for (c = 0; c < Dim; c++) {
          Pos[c] = 0.0;
          for (v = 0; v < Dim; v++) {
            Pos[c] += Lattice[c][v] * (double)Coords[v][s];
          }
        }
        DerProd = 1.0;
        for (t = 0; t < m; t++) {
          DerProd *= Pos[Direction[t]];
        }
        Sum += Ipow * DerProd * cexp(I * KdotR) * Blocks[b1][b2][s];
      }
      ASSERT(cabs(Array[i] - Sum) < Tolerance);
    }
  }

  // The band stride of the output is BandCount, not 2: a single non-zero block
  // stored at (BandId1, BandId2) = (1, 2) must appear at flat index
  // 1 + BandCount * 2 = 7 and nowhere else. A hard-coded stride of 2 or a
  // transposed pair of band indexes moves it.
  Probe = NewTBCrystal(Dim, Sites, Bands);
  ASSERT(Probe);

  Reference = 3.0 + 4.0 * I;
  ASSERT(TBCrystalSetTBHamiltonian(Probe, 1, 2, 3, Reference));

  Success =
      TBCrystalHamiltonian(Probe, Dim, Origin, 0, 9, ProbeArray, 2, ProbeShape);
  ASSERT_X(Success);
  ASSERT(ProbeShape[0] == Bands);
  ASSERT(ProbeShape[1] == Bands);
  for (i = 0; i < 9; i++) {
    if (i == 7) {
      ASSERT(cabs(ProbeArray[i] - Reference) < Tolerance);
    } else {
      ASSERT(cabs(ProbeArray[i]) < Tolerance);
    }
  }

  FreeTBCrystal(&Probe);
  ASSERT(!Probe);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

// The two-orbital chain of the realistic benchmark below: one lattice site per
// cell, carrying two orbitals, with on-site energies +Eps and -Eps, an
// intra-cell orbital coupling V and an inter-cell orbital coupling W,
//
//     H(k) = [ Eps                  V + W exp(+i k a) ]
//            [ V + W exp(-i k a)                -Eps ],
//
// with k = 2 pi x / a and x = KPoint[0]. This is the Bloch Hamiltonian of the
// Su-Schrieffer-Heeger (SSH) chain expressed in its two-orbital basis, whose
// spectrum
//
//     E_+/- (x) = +/- sqrt(Eps^2 + V^2 + W^2 + 2 V W cos(2 pi x))
//
// closes at x = 0 in the dimerized limit V = -W, where one of the two couplings
// vanishes, and stays gapped otherwise. The stored blocks satisfy
// H_{b2,b1}(-S) = conj(H_{b1,b2}(S)), which is what makes H(k) Hermitian.
static TBCrystal *NewTwoOrbitalChain(const double Eps,
                                     const double V,
                                     const double W,
                                     const double Lattice) {
  TBCrystal *Crystal = NULL;

  Crystal = NewTBCrystal(1, 3, 2);
  if (!Crystal) {
    ASSERT(Crystal);
    return NULL;
  }

  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, Lattice));

  // Site 0 sits at R = -a, site 1 at R = 0 and site 2 at R = +a.
  ASSERT(TBCrystalSetSiteVal(Crystal, 0, 0, -1));
  ASSERT(TBCrystalSetSiteVal(Crystal, 1, 0, 0));
  ASSERT(TBCrystalSetSiteVal(Crystal, 2, 0, 1));

  // On-site energies and the intra-cell coupling V at R = 0.
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 0, 1, Eps));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 1, 1, 1, -Eps));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 1, 1, V));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 1, 0, 1, V));

  // The inter-cell coupling W at R = +a and its Hermitian conjugate at R = -a.
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 0, 1, 2, W));
  ASSERT(TBCrystalSetTBHamiltonian(Crystal, 1, 0, 0, W));

  return Crystal;
}

TEST(TBCrystalBlochHamiltonian_TwoOrbitalChain) {
  TBCrystal     *Crystal   = NULL;
  const double   Pi        = acos(-1.0);
  const double   Tolerance = 1.0e-12;
  const double   Lattice   = 1.5;
  const double   Eps       = 0.35;
  const double   V         = 0.8;
  const double   W         = -0.6;
  const double   Points[7] = {0.0, 0.125, 0.25, 1.0 / 3.0, 0.5, -0.25, 0.75};
  double         KPoint[1];
  double complex Array[4];
  size_t         ArrayShape[2];
  double         Phase;
  double         Mid;
  double         Root;
  double         ExpectedRoot;
  size_t         i;
  bool           Success;

  Crystal = NewTwoOrbitalChain(Eps, V, W, Lattice);
  ASSERT_X(Crystal);

  for (i = 0; i < 7; i++) {
    KPoint[0] = Points[i];
    Success =
        TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 4, Array, 2, ArrayShape);
    ASSERT_X(Success);

    Phase = 2.0 * Pi * Points[i];

    // H(k) is Hermitian: the diagonal is real and the off-diagonal elements are
    // conjugates of each other. The two orbitals carry opposite on-site
    // energies, so the diagonal entries are each other's negative.
    ASSERT(fabs(cimag(Array[0])) < Tolerance);
    ASSERT(fabs(cimag(Array[3])) < Tolerance);
    ASSERT(fabs(creal(Array[3]) + creal(Array[0])) < Tolerance);
    ASSERT(cabs(Array[1] - conj(Array[2])) < Tolerance);

    // Element 2 is (BandId1, BandId2) = (0, 1) and element 1 is (1, 0).
    ASSERT(cabs(Array[2] - (V + W * cexp(I * Phase))) < Tolerance);
    ASSERT(cabs(Array[1] - (V + W * cexp(-I * Phase))) < Tolerance);

    // The splitting of a Hermitian 2x2 block around its mean is
    // sqrt(((h00 - h11)/2)^2 + h01 h10) and must reproduce the closed form.
    Mid          = 0.5 * (creal(Array[0]) + creal(Array[3]));
    Root         = sqrt(0.25 * (creal(Array[0]) - creal(Array[3])) *
                            (creal(Array[0]) - creal(Array[3])) +
                        creal(Array[1] * Array[2]));
    ExpectedRoot = sqrt(Eps * Eps + V * V + W * W + 2.0 * V * W * cos(Phase));

    // The bands are symmetric about zero, so the mean of the diagonal vanishes
    // and the splitting follows the analytic dispersion.
    ASSERT(fabs(Mid) < Tolerance);
    ASSERT(fabs(Root - ExpectedRoot) < Tolerance);
  }

  // Dimerized limit: with V = -W the two couplings are equal and opposite, the
  // orbitals decouple at x = 0 and the gap closes there, while it stays open
  // elsewhere.
  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);

  Crystal = NewTwoOrbitalChain(0.0, 0.7, -0.7, Lattice);
  ASSERT_X(Crystal);

  KPoint[0] = 0.0;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 4, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0]) < Tolerance);
  ASSERT(cabs(Array[1]) < Tolerance);
  ASSERT(cabs(Array[2]) < Tolerance);
  ASSERT(cabs(Array[3]) < Tolerance);

  // Half a period away the two couplings add in quadrature:
  // V + W exp(i pi/2) = 0.7 - 0.7 i.
  KPoint[0] = 0.25;
  Success =
      TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 4, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[2] - (0.7 - 0.7 * I)) < Tolerance);
  ASSERT(cabs(Array[1] - (0.7 + 0.7 * I)) < Tolerance);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalBlochHamiltonian_BadInput) {
  TBCrystal     *Crystal   = NULL;
  const size_t   Dim       = 2;
  const size_t   Bands     = 2;
  double         KPoint[3] = {0.125, -0.375, 0.5};
  double complex Array[16];
  size_t         ArrayShape[4];
  bool           Success;

  // KPoint must hold Dimensionality entries, ArrayShape Derivative + 2
  // entries and Array exactly BandCount^2 Dimensionality^Derivative
  // elements. Every mismatch is reported through the result, never through
  // a crash, and a rejection leaves the crystal reusable.

  Crystal = NewTBCrystal(Dim, 1, Bands);
  ASSERT(Crystal);

  // A KPoint size other than the crystal dimensionality is rejected.
  ASSERT(!TBCrystalHamiltonian(Crystal, 1, KPoint, 0, 4, Array, 2, ArrayShape));
  ASSERT(!TBCrystalHamiltonian(Crystal, 3, KPoint, 0, 4, Array, 2, ArrayShape));

  // Derivative = 0: two shape entries and four elements.
  Success =
      TBCrystalHamiltonian(Crystal, Dim, KPoint, 0, 4, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(ArrayShape[0] == Bands);
  ASSERT(ArrayShape[1] == Bands);

  // Too few or too many elements are rejected.
  ASSERT(
      !TBCrystalHamiltonian(Crystal, Dim, KPoint, 0, 3, Array, 2, ArrayShape));
  ASSERT(
      !TBCrystalHamiltonian(Crystal, Dim, KPoint, 0, 8, Array, 2, ArrayShape));

  // Derivative = 1: three shape entries and eight elements.
  ASSERT(
      !TBCrystalHamiltonian(Crystal, Dim, KPoint, 1, 8, Array, 2, ArrayShape));
  Success =
      TBCrystalHamiltonian(Crystal, Dim, KPoint, 1, 8, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(ArrayShape[2] == Dim);

  // Derivative = 2: sixteen elements and four shape entries.
  ASSERT(
      !TBCrystalHamiltonian(Crystal, Dim, KPoint, 2, 8, Array, 4, ArrayShape));
  ASSERT(
      !TBCrystalHamiltonian(Crystal, Dim, KPoint, 2, 16, Array, 3, ArrayShape));

  // A rank that cannot be represented is rejected up front, instead of
  // wrapping around to a small, accidentally valid rank.
  ASSERT(!TBCrystalHamiltonian(Crystal, Dim, KPoint, SIZE_MAX, 4, Array, 2,
                               ArrayShape));

  // A missing crystal or a missing buffer is reported, never dereferenced.
  ASSERT(!TBCrystalHamiltonian(NULL, Dim, KPoint, 0, 4, Array, 2, ArrayShape));
  ASSERT(!TBCrystalHamiltonian(Crystal, Dim, NULL, 0, 4, Array, 2, ArrayShape));
  ASSERT(
      !TBCrystalHamiltonian(Crystal, Dim, KPoint, 0, 4, NULL, 2, ArrayShape));
  ASSERT(!TBCrystalHamiltonian(Crystal, Dim, KPoint, 0, 4, Array, 2, NULL));

  // The rejections left no state behind: a well-formed call still succeeds.
  Success =
      TBCrystalHamiltonian(Crystal, Dim, KPoint, 0, 4, Array, 2, ArrayShape);
  ASSERT_X(Success);
  ASSERT(ArrayShape[0] == Bands);
  ASSERT(ArrayShape[1] == Bands);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_SUITE(TBCIFunctionalBlochHamiltonianSuite,
           TBCrystalBlochHamiltonian_ChainDispersion,
           TBCrystalBlochHamiltonian_ChainDerivatives,
           TBCrystalBlochHamiltonian_Layout,
           TBCrystalBlochHamiltonian_StridedLayout,
           TBCrystalBlochHamiltonian_TwoOrbitalChain,
           TBCrystalBlochHamiltonian_BadInput);

// TBPositionOperator evaluates the Derivative-th Cartesian k-derivative of the
// Bloch Berry connection,
//
//     d^m A_c(k)/dk^m = d^m/dk^m sum_S A(b1, b2, c, S) exp(i k.R_S),
//
// and reports the shape of the result. The Cartesian component c labels the
// stored connection and is never differentiated, so the result carries one
// component index followed by one direction index per derivative order: its
// rank is Derivative + 3. The accessors of the stored Berry connection share
// the TBCrystal prefix, hence the Bloch qualifier.

TEST(TBCrystalBlochPositionOperator_ChainDispersion) {
  TBCrystal     *Crystal          = NULL;
  const double   Pi               = acos(-1.0);
  const double   Tolerance        = 1.0e-12;
  const double   Lattice          = 2.0;
  const double   ReducedPoints[7] = {0.0, 0.125, 0.25, 1.0 / 3.0,
                                     0.5, -0.25, -0.5};
  double         KPoint[1];
  double complex Array[1];
  double complex Reference;
  size_t         ArrayShape[3];
  size_t         i;
  bool           Success;

  // A one-dimensional, one-band chain with the sites at R = -a, R = 0 and
  // R = +a,
  //
  //     ... -- o -- o -- o -- ...
  //            R=-a  R=0  R=+a
  //
  // carrying A(S) = R(S), so that
  //
  //     A(k) = -a exp(-i k a) + a exp(+i k a) = 2 i a sin(2 pi x),
  //
  // with k = 2 pi x / a and x = KPoint[0]. The result is purely imaginary and
  // odd in x: the sign pins the exp(+i k.R_S) phase convention and the
  // vanishing real part pins that the Berry connection is summed on its own,
  // without mixing in the Hamiltonian.

  Crystal = NewTBCrystal(1, 3, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, Lattice));
  ASSERT(TBCrystalSetSiteVal(Crystal, 0, 0, -1));
  ASSERT(TBCrystalSetSiteVal(Crystal, 1, 0, 0));
  ASSERT(TBCrystalSetSiteVal(Crystal, 2, 0, 1));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 0, -Lattice));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 1, 0, 0.0));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 2, 0, Lattice));

  // The analytic connection is recovered across the reduced zone, and the shape
  // reports the component index on top of the band pair.
  for (i = 0; i < 7; i++) {
    KPoint[0]     = ReducedPoints[i];
    ArrayShape[0] = 0;
    ArrayShape[1] = 0;
    ArrayShape[2] = 0;
    Success =
        TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
    ASSERT_X(Success);
    ASSERT(ArrayShape[0] == 1);
    ASSERT(ArrayShape[1] == 1);
    ASSERT(ArrayShape[2] == 1);
    Reference = 2.0 * Lattice * sin(2.0 * Pi * ReducedPoints[i]) * I;
    ASSERT(cabs(Array[0] - Reference) < Tolerance);
    ASSERT(fabs(creal(Array[0])) < Tolerance);
  }

  // Nodes at x = 0 and x = 1/2, extrema +2 i a at x = 1/4 and -2 i a at
  // x = -1/4.
  KPoint[0] = 0.0;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0]) < Tolerance);

  KPoint[0] = 0.25;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - 2.0 * Lattice * I) < Tolerance);

  KPoint[0] = -0.25;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] + 2.0 * Lattice * I) < Tolerance);

  KPoint[0] = 0.5;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0]) < Tolerance);

  // The phase is periodic in every reduced coordinate with period 1, so
  // replicas of the same point must agree.
  KPoint[0] = 0.2;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  Reference = Array[0];

  KPoint[0] = 1.2;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Reference) < Tolerance);

  KPoint[0] = -1.8;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Reference) < Tolerance);

  // x is a reduced coordinate, so the zeroth derivative does not depend on
  // the length of the direct lattice vector: only k a enters the phase.
  KPoint[0] = 0.3;
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  Reference = Array[0];

  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, 3.7));
  Success = TBPositionOperator(Crystal, 1, KPoint, 0, 1, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Reference) < Tolerance);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalBlochPositionOperator_ChainDerivatives) {
  TBCrystal   *Crystal          = NULL;
  const double Pi               = acos(-1.0);
  const double Tolerance        = 1.0e-12;
  const double Lattice          = 2.0;
  const double ReducedPoints[5] = {0.0, 0.125, 0.25, 0.5, -0.375};
  // A complex weight makes every derivative carry a real and an imaginary
  // part, so a mixed-up i^m factor cannot hide behind a vanishing component.
  const double complex Prefactor = 1.5 - 2.5 * I;
  double               KPoint[1];
  double complex       Array[1];
  double complex       Reference;
  double complex       Scaled;
  size_t               ArrayShape[7];
  size_t               i;
  size_t               m;
  size_t               t;
  bool                 Success;

  // The chain of the dispersion benchmark carries P(S) = (n a) P0 with a
  // complex weight P0, hence
  //
  //     P(k)         = 2 i a P0 sin(k a),
  //     d^m P / dk^m = 2 i a^(m+1) P0 sin(2 pi x + m pi / 2),
  //
  // with k = 2 pi x / a. The reference pins the chain rule (powers of a), the
  // i^m factor and the (m + 3)-ranked shape of the result at once.

  Crystal = NewTBCrystal(1, 3, 1);
  ASSERT(Crystal);
  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, Lattice));
  ASSERT(TBCrystalSetSiteVal(Crystal, 0, 0, -1));
  ASSERT(TBCrystalSetSiteVal(Crystal, 1, 0, 0));
  ASSERT(TBCrystalSetSiteVal(Crystal, 2, 0, 1));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 0, 0,
                                        -Lattice * Prefactor));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 0, 1, 0, 0.0));
  ASSERT(
      TBCrystalSetTBPositionOperator(Crystal, 0, 0, 2, 0, Lattice * Prefactor));

  for (m = 1; m <= 4; m++) {
    for (i = 0; i < 5; i++) {
      KPoint[0] = ReducedPoints[i];
      for (t = 0; t < m + 3; t++) {
        ArrayShape[t] = 0;
      }
      Success = TBPositionOperator(Crystal, 1, KPoint, m, 1, Array, m + 3,
                                   ArrayShape);
      ASSERT_X(Success);

      // BandCount, BandCount, the undifferentiated component and then one
      // entry per derivative direction.
      ASSERT(ArrayShape[0] == 1);
      ASSERT(ArrayShape[1] == 1);
      ASSERT(ArrayShape[2] == 1);
      for (t = 0; t < m; t++) {
        ASSERT(ArrayShape[3 + t] == 1);
      }

      Reference = 2.0 * I * Prefactor * pow(Lattice, (double)(m + 1)) *
                  sin(2.0 * Pi * ReducedPoints[i] + 0.5 * (double)m * Pi);
      ASSERT(cabs(Array[0] - Reference) < Tolerance);
    }
  }

  // At x = 0 the odd derivatives alternate between +2 i a^(m+1) P0 and
  // -2 i a^(m+1) P0, while the even ones vanish.
  KPoint[0] = 0.0;
  Success = TBPositionOperator(Crystal, 1, KPoint, 1, 1, Array, 4, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - 2.0 * I * Prefactor * Lattice * Lattice) < Tolerance);

  Success = TBPositionOperator(Crystal, 1, KPoint, 2, 1, Array, 5, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0]) < Tolerance);

  Success = TBPositionOperator(Crystal, 1, KPoint, 3, 1, Array, 6, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] + 2.0 * I * Prefactor * pow(Lattice, 4.0)) < Tolerance);

  Success = TBPositionOperator(Crystal, 1, KPoint, 4, 1, Array, 7, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0]) < Tolerance);

  // Every derivative order picks up one factor R_c = a n per order, so
  // rescaling the direct lattice rescales the m-th derivative by (a'/a)^m.
  // The stored blocks are left untouched: a mismatch here would mean that the
  // integer site offsets, rather than their Cartesian positions, reached the
  // derivative.
  KPoint[0] = 0.375;
  Success = TBPositionOperator(Crystal, 1, KPoint, 2, 1, Array, 5, ArrayShape);
  ASSERT_X(Success);
  Scaled = Array[0];

  ASSERT(TBCrystalSetDirectLatticeVal(Crystal, 0, 0, 3.7));
  Success = TBPositionOperator(Crystal, 1, KPoint, 2, 1, Array, 5, ArrayShape);
  ASSERT_X(Success);
  ASSERT(cabs(Array[0] - Scaled * pow(3.7 / Lattice, 2.0)) < Tolerance);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalBlochPositionOperator_Layout) {
  TBCrystal   *Crystal   = NULL;
  TBCrystal   *Skewed    = NULL;
  const double Pi        = acos(-1.0);
  const double Tolerance = 1.0e-12;
  const size_t Dim       = 2;
  const size_t Bands     = 2;
  const size_t Sites     = 4;
  const size_t MaxRank   = 5;
  // Lattice[ComponentId][VectorId]: column j holds the vector a_j.
  const double   Lattice[2][2] = {{1.0, 0.5}, {0.0, 2.0}};
  const long int Coords[2][4]  = {{0, 1, 0, 1}, {0, 0, 1, 1}};
  const double   Origin[2]     = {0.0, 0.0};
  double         KPoint[2]     = {0.125, -0.375};
  double complex Blocks[2][2][2][4];
  double complex Array[32];
  double complex SkewedArray[8];
  double complex Reference;
  double complex Sum;
  double complex Ipow;
  double complex Expected;
  size_t         ArrayShape[5];
  size_t         SkewedShape[3];
  size_t         Direction[2];
  size_t         b1;
  size_t         b2;
  size_t         s;
  size_t         c;
  size_t         m;
  size_t         t;
  size_t         i;
  size_t         idx;
  size_t         Elements;
  double         KdotR;
  double         DerProd;
  double         Pos[2];
  bool           Success;

  // Two dimensions, two bands and four sites on a non-orthogonal lattice, so
  // that the Cartesian position of site s is
  //
  //     R = a1 Coords[0][s] + a2 Coords[1][s],
  //
  // with a1 = (1, 0) and a2 = (1/2, 2). Every block depends on the Cartesian
  // component c, and the blocks are deliberately non-Hermitian and
  // asymmetric under BandId1 <-> BandId2, so a dropped, transposed or
  // hermitized read, a component index swapped with a band index and a
  // component index used as a derivative direction all change the result.

  Crystal = NewTBCrystal(Dim, Sites, Bands);
  ASSERT(Crystal);

  for (c = 0; c < Dim; c++) {
    for (s = 0; s < Dim; s++) {
      ASSERT(TBCrystalSetDirectLatticeVal(Crystal, s, c, Lattice[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (c = 0; c < Dim; c++) {
      ASSERT(TBCrystalSetSiteVal(Crystal, s, c, Coords[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (c = 0; c < Dim; c++) {
      for (b2 = 0; b2 < Bands; b2++) {
        for (b1 = 0; b1 < Bands; b1++) {
          Reference = (double)(b1 + 10 * b2 + 100 * c + 1000 * s) +
                      (-(double)(b1 + 3 * b2 + 7 * c + 11 * s) - 0.25) * I;
          Blocks[b1][b2][c][s] = Reference;
          ASSERT(
              TBCrystalSetTBPositionOperator(Crystal, b1, b2, s, c, Reference));
        }
      }
    }
  }

  for (m = 0; m <= 2; m++) {
    Elements = Bands * Bands * Dim;
    for (t = 0; t < m; t++) {
      Elements *= Dim;
    }

    // Only the reported extents are written; the rest of the buffer is left
    // untouched.
    for (t = 0; t < MaxRank; t++) {
      ArrayShape[t] = (size_t)-1;
    }

    Success = TBPositionOperator(Crystal, Dim, KPoint, m, Elements, Array,
                                 m + 3, ArrayShape);
    ASSERT_X(Success);
    ASSERT(ArrayShape[0] == Bands);
    ASSERT(ArrayShape[1] == Bands);
    ASSERT(ArrayShape[2] == Dim);
    for (t = 0; t < m; t++) {
      ASSERT(ArrayShape[3 + t] == Dim);
    }
    for (t = m + 3; t < MaxRank; t++) {
      ASSERT(ArrayShape[t] == (size_t)-1);
    }

    switch (m % 4) {
    case 0:
      Ipow = 1.0;
      break;
    case 1:
      Ipow = I;
      break;
    default:
      Ipow = -1.0;
      break;
    }

    for (i = 0; i < Elements; i++) {
      // Column-major: BandId1 is the fastest index, then BandId2, then the
      // Cartesian component, then the direction of each successive
      // derivative.
      idx = i;
      b1  = idx % Bands;
      idx = idx / Bands;
      b2  = idx % Bands;
      idx = idx / Bands;
      c   = idx % Dim;
      idx = idx / Dim;
      for (t = 0; t < m; t++) {
        Direction[t] = idx % Dim;
        idx          = idx / Dim;
      }

      Sum = 0.0;
      for (s = 0; s < Sites; s++) {
        KdotR   = 2.0 * Pi *
                  ((double)Coords[0][s] * KPoint[0] +
                   (double)Coords[1][s] * KPoint[1]);
        Pos[0]  = Lattice[0][0] * (double)Coords[0][s] +
                  Lattice[0][1] * (double)Coords[1][s];
        Pos[1]  = Lattice[1][0] * (double)Coords[0][s] +
                  Lattice[1][1] * (double)Coords[1][s];
        DerProd = 1.0;
        for (t = 0; t < m; t++) {
          DerProd *= Pos[Direction[t]];
        }
        Sum += Ipow * DerProd * cexp(I * KdotR) * Blocks[b1][b2][c][s];
      }
      ASSERT(cabs(Array[i] - Sum) < Tolerance);
    }
  }

  // (BandId1, BandId2, ComponentId) = (0, 1, 1) is returned as stored: no
  // hermitian conjugation, and the component index is the slowest of the
  // three, so its flat index is 0 + 2 * 1 + 4 * 1 = 6.
  Skewed   = NewTBCrystal(Dim, 1, Bands);
  Expected = 1.0 + 1.0 * I;
  ASSERT(Skewed);
  ASSERT(TBCrystalSetTBPositionOperator(Skewed, 0, 1, 0, 1, Expected));

  Success = TBPositionOperator(Skewed, Dim, Origin, 0, 8, SkewedArray, 3,
                               SkewedShape);
  ASSERT_X(Success);
  ASSERT(SkewedShape[0] == Bands);
  ASSERT(SkewedShape[1] == Bands);
  ASSERT(SkewedShape[2] == Dim);
  for (i = 0; i < 8; i++) {
    if (i == 6) {
      ASSERT(cabs(SkewedArray[i] - Expected) < Tolerance);
    } else {
      ASSERT(cabs(SkewedArray[i]) < Tolerance);
    }
  }

  // Element 1 is (BandId1, BandId2, ComponentId) = (1, 0, 0).
  Expected = 2.0 - 1.0 * I;
  ASSERT(TBCrystalSetTBPositionOperator(Skewed, 1, 0, 0, 0, Expected));
  Success = TBPositionOperator(Skewed, Dim, Origin, 0, 8, SkewedArray, 3,
                               SkewedShape);
  ASSERT_X(Success);
  ASSERT(cabs(SkewedArray[1] - Expected) < Tolerance);
  ASSERT(cabs(SkewedArray[6] - (1.0 + 1.0 * I)) < Tolerance);

  FreeTBCrystal(&Skewed);
  ASSERT(!Skewed);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

// The Berry connection of the strided layout: three bands, three dimensions
// and five sites, so that the band stride 3 and the component stride 3 differ
// from the 2 of every other layout test, and the second band stride 9 differs
// from the component stride. A mix-up of the two strides, a hard-coded 2 and a
// component index used as a derivative direction all change the result here,
// whereas they can cancel when BandCount = Dimensionality = 2.

TEST(TBCrystalBlochPositionOperator_StridedLayout) {
  TBCrystal   *Crystal   = NULL;
  TBCrystal   *Probe     = NULL;
  const double Pi        = acos(-1.0);
  const double Tolerance = 1.0e-12;
  const size_t Dim       = 3;
  const size_t Bands     = 3;
  const size_t Sites     = 5;
  const size_t MaxRank   = 5;
  // Lattice[ComponentId][VectorId]: column j holds the vector a_j. The three
  // vectors are neither orthogonal nor symmetric.
  const double   Lattice[3][3] = {{1.0, 0.25, -0.5},
                                  {0.0, 1.5, 0.75},
                                  {0.0, 0.0, 2.0}};
  const long int Coords[3][5]  = {{0, 1, 0, 0, 1},
                                  {0, 0, 1, 0, -1},
                                  {0, 0, 0, 1, 1}};
  const double   Origin[3]     = {0.0, 0.0, 0.0};
  double         KPoint[3]     = {0.125, 0.3, -0.4};
  double complex Blocks[3][3][3][5];
  double complex Array[243];
  double complex ProbeArray[27];
  double complex Reference;
  double complex Sum;
  double complex Ipow;
  size_t         ArrayShape[5];
  size_t         ProbeShape[3];
  size_t         Direction[2];
  size_t         b1;
  size_t         b2;
  size_t         s;
  size_t         c;
  size_t         m;
  size_t         t;
  size_t         v;
  size_t         i;
  size_t         idx;
  size_t         Elements;
  double         KdotR;
  double         DerProd;
  double         Pos[3];
  bool           Success;

  Crystal = NewTBCrystal(Dim, Sites, Bands);
  ASSERT(Crystal);

  for (c = 0; c < Dim; c++) {
    for (s = 0; s < Dim; s++) {
      ASSERT(TBCrystalSetDirectLatticeVal(Crystal, s, c, Lattice[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (c = 0; c < Dim; c++) {
      ASSERT(TBCrystalSetSiteVal(Crystal, s, c, Coords[c][s]));
    }
  }

  // Every block depends on the Cartesian component, and both parts are
  // distinct across every index, so a swapped component, a swapped band pair
  // and a hermitized read all change the result.
  for (s = 0; s < Sites; s++) {
    for (c = 0; c < Dim; c++) {
      for (b2 = 0; b2 < Bands; b2++) {
        for (b1 = 0; b1 < Bands; b1++) {
          Reference =
              (double)(1 + b1 + 3 * b2 + 10 * c + 100 * s) +
              (-(double)(5 + 2 * b1 + 4 * b2 + 7 * c + 11 * s) - 0.25) * I;
          Blocks[b1][b2][c][s] = Reference;
          ASSERT(
              TBCrystalSetTBPositionOperator(Crystal, b1, b2, s, c, Reference));
        }
      }
    }
  }

  for (m = 0; m <= 2; m++) {
    Elements = Bands * Bands * Dim;
    for (t = 0; t < m; t++) {
      Elements *= Dim;
    }

    for (t = 0; t < MaxRank; t++) {
      ArrayShape[t] = (size_t)-1;
    }

    Success = TBPositionOperator(Crystal, Dim, KPoint, m, Elements, Array,
                                 m + 3, ArrayShape);
    ASSERT_X(Success);
    ASSERT(ArrayShape[0] == Bands);
    ASSERT(ArrayShape[1] == Bands);
    ASSERT(ArrayShape[2] == Dim);
    for (t = 0; t < m; t++) {
      ASSERT(ArrayShape[3 + t] == Dim);
    }
    for (t = m + 3; t < MaxRank; t++) {
      ASSERT(ArrayShape[t] == (size_t)-1);
    }

    switch (m % 4) {
    case 0:
      Ipow = 1.0;
      break;
    case 1:
      Ipow = I;
      break;
    default:
      Ipow = -1.0;
      break;
    }

    for (i = 0; i < Elements; i++) {
      // Column-major: BandId1 is the fastest index, then BandId2, then the
      // Cartesian component, then the direction of each successive derivative.
      idx = i;
      b1  = idx % Bands;
      idx = idx / Bands;
      b2  = idx % Bands;
      idx = idx / Bands;
      c   = idx % Dim;
      idx = idx / Dim;
      for (t = 0; t < m; t++) {
        Direction[t] = idx % Dim;
        idx          = idx / Dim;
      }

      Sum = 0.0;
      for (s = 0; s < Sites; s++) {
        KdotR = 2.0 * Pi *
                ((double)Coords[0][s] * KPoint[0] +
                 (double)Coords[1][s] * KPoint[1] +
                 (double)Coords[2][s] * KPoint[2]);
        for (t = 0; t < Dim; t++) {
          Pos[t] = 0.0;
          for (v = 0; v < Dim; v++) {
            Pos[t] += Lattice[t][v] * (double)Coords[v][s];
          }
        }
        DerProd = 1.0;
        for (t = 0; t < m; t++) {
          DerProd *= Pos[Direction[t]];
        }
        Sum += Ipow * DerProd * cexp(I * KdotR) * Blocks[b1][b2][c][s];
      }
      ASSERT(cabs(Array[i] - Sum) < Tolerance);
    }
  }

  // The output strides are BandCount = 3 for BandId1, BandCount = 3 for
  // BandId2 and BandCount^2 = 9 for the Cartesian component: a single non-zero
  // block stored at (BandId1, BandId2, ComponentId) = (1, 2, 1) must appear at
  // flat index 1 + 3 * 2 + 9 * 1 = 16 and nowhere else.
  Probe = NewTBCrystal(Dim, Sites, Bands);
  ASSERT(Probe);

  Reference = 3.0 + 4.0 * I;
  ASSERT(TBCrystalSetTBPositionOperator(Probe, 1, 2, 3, 1, Reference));

  Success =
      TBPositionOperator(Probe, Dim, Origin, 0, 27, ProbeArray, 3, ProbeShape);
  ASSERT_X(Success);
  ASSERT(ProbeShape[0] == Bands);
  ASSERT(ProbeShape[1] == Bands);
  ASSERT(ProbeShape[2] == Dim);
  for (i = 0; i < 27; i++) {
    if (i == 16) {
      ASSERT(cabs(ProbeArray[i] - Reference) < Tolerance);
    } else {
      ASSERT(cabs(ProbeArray[i]) < Tolerance);
    }
  }

  FreeTBCrystal(&Probe);
  ASSERT(!Probe);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

// The Berry connection of the two-orbital chain: the stored connection is the
// cell position A_{b1,b2,c}(S) = R_c(S) H_{b1,b2}(S), whose Bloch transform is
// the k-derivative identity
//
//     A_c(k) = -i dH(k)/dk_c.
//
// Only the two inter-cell blocks have a non-zero position, so in this chain
// A(k) is purely off-diagonal and anti-Hermitian, and every derivative order m
// picks up one more factor i R_c.
static TBCrystal *NewTwoOrbitalChainPosition(const double Eps,
                                             const double V,
                                             const double W,
                                             const double Lattice) {
  TBCrystal *Crystal = NULL;

  Crystal = NewTwoOrbitalChain(Eps, V, W, Lattice);
  if (!Crystal) {
    return NULL;
  }

  // P(0) vanishes because R(0) = 0; P(+a) and P(-a) carry one factor R = +/-a.
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 0, 1, 2, 0, Lattice * W));
  ASSERT(TBCrystalSetTBPositionOperator(Crystal, 1, 0, 0, 0, -Lattice * W));

  return Crystal;
}

TEST(TBCrystalBlochPositionOperator_TwoOrbitalChain) {
  TBCrystal     *Crystal   = NULL;
  TBCrystal     *Chain     = NULL;
  const double   Pi        = acos(-1.0);
  const double   Tolerance = 1.0e-12;
  const double   Lattice   = 1.5;
  const double   Eps       = 0.35;
  const double   V         = 0.8;
  const double   W         = -0.6;
  const double   Points[4] = {0.0, 0.125, 0.25, -0.375};
  double         KPoint[1];
  double complex Array[4];
  double complex Hamiltonian[4];
  double complex Reference;
  double complex Ipow;
  size_t         ArrayShape[5];
  size_t         m;
  size_t         i;
  double         Phase;
  double         Sign;
  bool           Success;

  Crystal = NewTwoOrbitalChainPosition(Eps, V, W, Lattice);
  ASSERT_X(Crystal);

  for (i = 0; i < 4; i++) {
    KPoint[0] = Points[i];
    Phase     = 2.0 * Pi * Points[i];

    for (m = 0; m <= 2; m++) {
      Success = TBPositionOperator(Crystal, 1, KPoint, m, 4, Array, m + 3,
                                   ArrayShape);
      ASSERT_X(Success);
      ASSERT(ArrayShape[0] == 2);
      ASSERT(ArrayShape[1] == 2);
      ASSERT(ArrayShape[2] == 1);

      switch (m % 4) {
      case 0:
        Ipow = 1.0;
        break;
      case 1:
        Ipow = I;
        break;
      case 2:
        Ipow = -1.0;
        break;
      default:
        Ipow = -I;
        break;
      }

      // Column-major with Dimensionality = 1: (BandId1, BandId2) = (0, 1) is
      // element 2 and (1, 0) is element 1 at every order. The two inter-cell
      // blocks sit at opposite positions, so the (+a) value is
      // i^m a^(m + 1) W exp(+i k a) and its (-a) partner differs by the sign of
      // the position to the power m + 1.
      Reference = Ipow * pow(Lattice, (double)(m + 1)) * W;
      Sign      = (m % 2 == 0) ? -1.0 : 1.0;
      ASSERT(cabs(Array[2] - Reference * cexp(I * Phase)) < Tolerance);
      ASSERT(cabs(Array[1] - Sign * Reference * cexp(-I * Phase)) < Tolerance);
      ASSERT(cabs(Array[0]) < Tolerance);
      ASSERT(cabs(Array[3]) < Tolerance);

      // Every order is anti-Hermitian, as the derivatives of the anti-Hermitian
      // -i dH/dk of a Hermitian H must be.
      ASSERT(cabs(Array[1] + conj(Array[2])) < Tolerance);
    }
  }

  // The identity A_c(k) = -i dH(k)/dk_c holds exactly: the m = 1 output of the
  // Hamiltonian kernel, multiplied by -i, is the m = 0 output of the connection
  // kernel of the same chain.
  Chain = NewTwoOrbitalChain(Eps, V, W, Lattice);
  ASSERT_X(Chain);

  for (i = 0; i < 4; i++) {
    KPoint[0] = Points[i];

    Success = TBCrystalHamiltonian(Chain, 1, KPoint, 1, 4, Hamiltonian, 3,
                                   ArrayShape);
    ASSERT_X(Success);
    Success =
        TBPositionOperator(Crystal, 1, KPoint, 0, 4, Array, 3, ArrayShape);
    ASSERT_X(Success);

    for (m = 0; m < 4; m++) {
      ASSERT(cabs(Array[m] + I * Hamiltonian[m]) < Tolerance);
    }
  }

  FreeTBCrystal(&Chain);
  ASSERT(!Chain);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalBlochPositionOperator_CrossKernelConsistency) {
  TBCrystal     *Crystal       = NULL;
  const double   Tolerance     = 1.0e-12;
  const size_t   Dim           = 2;
  const size_t   Bands         = 2;
  const size_t   Sites         = 4;
  const double   Lattice[2][2] = {{1.0, 0.5}, {0.0, 2.0}};
  const long int Coords[2][4]  = {{0, 1, 0, 1}, {0, 0, 1, 1}};
  double         KPoint[2]     = {0.125, -0.375};
  double complex Hamiltonian0[4];
  double complex Hamiltonian1[8];
  double complex Hamiltonian2[16];
  double complex Position0[8];
  double complex Position1[16];
  double complex Blocks[2][2][4];
  double complex Reference;
  size_t         ArrayShape[5];
  size_t         b1;
  size_t         b2;
  size_t         s;
  size_t         c;
  size_t         i;
  bool           Success;

  // A site-resolved Berry connection is A(b1, b2, c, S) = R_c(S) H(b1, b2, S),
  // and each k-derivative of the phase contributes i R, hence
  //
  //     A_c(k)    = sum_S R_c H exp(i k.R_S) = -i dH(k)/dk_c,
  //     dA_c/dk_d = sum_S i R_c R_d H exp(i k.R_S) = -i d^2H(k)/(dk_c dk_d).
  //
  // Both kernels lay out their result as [b1][b2][c]([d])..., so the
  // identities are elementwise comparisons between results of equal length,
  // and they pin the direction order of the derivative slots of one kernel
  // against the other.

  Crystal = NewTBCrystal(Dim, Sites, Bands);
  ASSERT(Crystal);

  for (c = 0; c < Dim; c++) {
    for (s = 0; s < Dim; s++) {
      ASSERT(TBCrystalSetDirectLatticeVal(Crystal, s, c, Lattice[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (c = 0; c < Dim; c++) {
      ASSERT(TBCrystalSetSiteVal(Crystal, s, c, Coords[c][s]));
    }
  }

  for (s = 0; s < Sites; s++) {
    for (b2 = 0; b2 < Bands; b2++) {
      for (b1 = 0; b1 < Bands; b1++) {
        Blocks[b1][b2][s] = (double)(b1 + 10 * b2 + 100 * s) +
                            (1000.0 + (double)(b1 + 10 * b2 + 100 * s)) * I;
        ASSERT(
            TBCrystalSetTBHamiltonian(Crystal, b1, b2, s, Blocks[b1][b2][s]));
        for (c = 0; c < Dim; c++) {
          Reference = (Lattice[c][0] * (double)Coords[0][s] +
                       Lattice[c][1] * (double)Coords[1][s]) *
                      Blocks[b1][b2][s];
          ASSERT(
              TBCrystalSetTBPositionOperator(Crystal, b1, b2, s, c, Reference));
        }
      }
    }
  }

  Success = TBCrystalHamiltonian(Crystal, Dim, KPoint, 0, 4, Hamiltonian0, 2,
                                 ArrayShape);
  ASSERT_X(Success);
  Success = TBCrystalHamiltonian(Crystal, Dim, KPoint, 1, 8, Hamiltonian1, 3,
                                 ArrayShape);
  ASSERT_X(Success);
  Success = TBCrystalHamiltonian(Crystal, Dim, KPoint, 2, 16, Hamiltonian2, 4,
                                 ArrayShape);
  ASSERT_X(Success);

  Success =
      TBPositionOperator(Crystal, Dim, KPoint, 0, 8, Position0, 3, ArrayShape);
  ASSERT_X(Success);
  Success =
      TBPositionOperator(Crystal, Dim, KPoint, 1, 16, Position1, 4, ArrayShape);
  ASSERT_X(Success);

  // The identity compares two computed quantities, so neither side may be
  // identically zero.
  ASSERT(cabs(Hamiltonian0[3]) > Tolerance);
  ASSERT(cabs(Position0[3]) > Tolerance);

  // P_c(k) = -i dH/dk_c.
  for (i = 0; i < 8; i++) {
    ASSERT(cabs(Position0[i] + I * Hamiltonian1[i]) < Tolerance);
  }

  // dP_c/dk_d = -i d^2H/(dk_c dk_d).
  for (i = 0; i < 16; i++) {
    ASSERT(cabs(Position1[i] + I * Hamiltonian2[i]) < Tolerance);
  }

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST(TBCrystalBlochPositionOperator_BadInput) {
  TBCrystal     *Crystal   = NULL;
  const size_t   Dim       = 2;
  const size_t   Bands     = 2;
  double         KPoint[3] = {0.125, -0.375, 0.5};
  double complex Array[32];
  size_t         ArrayShape[5];
  bool           Success;

  // KPoint must hold Dimensionality entries, ArrayShape Derivative + 3
  // entries and Array exactly BandCount^2 Dimensionality^(Derivative + 1)
  // elements. Every mismatch is reported through the result, never through a
  // crash, and a rejection leaves the crystal reusable.

  Crystal = NewTBCrystal(Dim, 1, Bands);
  ASSERT(Crystal);

  // A KPoint size other than the crystal dimensionality is rejected.
  ASSERT(!TBPositionOperator(Crystal, 1, KPoint, 0, 8, Array, 3, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, 3, KPoint, 0, 8, Array, 3, ArrayShape));

  // Derivative = 0: three shape entries and eight elements.
  Success =
      TBPositionOperator(Crystal, Dim, KPoint, 0, 8, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(ArrayShape[0] == Bands);
  ASSERT(ArrayShape[1] == Bands);
  ASSERT(ArrayShape[2] == Dim);

  // Too few or too many elements are rejected, and so is the two-entry shape
  // of the Hamiltonian kernel: the component index is mandatory.
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, 0, 7, Array, 3, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, 0, 9, Array, 3, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, 0, 8, Array, 2, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, 0, 8, Array, 4, ArrayShape));

  // Derivative = 1: sixteen elements and four shape entries.
  ASSERT(
      !TBPositionOperator(Crystal, Dim, KPoint, 1, 16, Array, 3, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, 1, 8, Array, 4, ArrayShape));
  Success =
      TBPositionOperator(Crystal, Dim, KPoint, 1, 16, Array, 4, ArrayShape);
  ASSERT_X(Success);
  ASSERT(ArrayShape[3] == Dim);

  // Derivative = 2: thirty-two elements and five shape entries.
  ASSERT(
      !TBPositionOperator(Crystal, Dim, KPoint, 2, 32, Array, 4, ArrayShape));
  ASSERT(
      !TBPositionOperator(Crystal, Dim, KPoint, 2, 16, Array, 5, ArrayShape));
  Success =
      TBPositionOperator(Crystal, Dim, KPoint, 2, 32, Array, 5, ArrayShape);
  ASSERT_X(Success);
  ASSERT(ArrayShape[4] == Dim);

  // A rank that cannot be represented is rejected up front, instead of
  // wrapping around to a small, accidentally valid rank.
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, SIZE_MAX, 8, Array, 3,
                             ArrayShape));

  // A missing crystal or a missing buffer is reported, never dereferenced.
  ASSERT(!TBPositionOperator(NULL, Dim, KPoint, 0, 8, Array, 3, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, Dim, NULL, 0, 8, Array, 3, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, 0, 8, NULL, 3, ArrayShape));
  ASSERT(!TBPositionOperator(Crystal, Dim, KPoint, 0, 8, Array, 3, NULL));

  // The rejections left no state behind: a well-formed call still succeeds.
  Success =
      TBPositionOperator(Crystal, Dim, KPoint, 0, 8, Array, 3, ArrayShape);
  ASSERT_X(Success);
  ASSERT(ArrayShape[0] == Bands);
  ASSERT(ArrayShape[1] == Bands);
  ASSERT(ArrayShape[2] == Dim);

  FreeTBCrystal(&Crystal);
  ASSERT(!Crystal);
}

TEST_SUITE(TBCIFunctionalBlochPositionOperatorSuite,
           TBCrystalBlochPositionOperator_ChainDispersion,
           TBCrystalBlochPositionOperator_ChainDerivatives,
           TBCrystalBlochPositionOperator_Layout,
           TBCrystalBlochPositionOperator_StridedLayout,
           TBCrystalBlochPositionOperator_TwoOrbitalChain,
           TBCrystalBlochPositionOperator_CrossKernelConsistency,
           TBCrystalBlochPositionOperator_BadInput);
