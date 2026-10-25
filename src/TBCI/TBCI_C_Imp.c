#include "src/Core/Core.h"
#include "src/MAC/MultidimensionalArrayContainers.h"
#include "src/TBCI/TightBindingCrystalInterpolation.h"

struct TBCrystal {
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
};

// Unsafe routine. Does not check Crystal for NULL. Acceptable because caller
// verifies.
const double *TBCI_InternalGetDirectLatticePtr(const TBCrystal *Crystal) {
  return Crystal->DirectLattice;
}

// Unsafe routine. Does not check Crystal for NULL. Acceptable because caller
// verifies.
const long int *TBCI_InternalGetSiteCoordsPtr(const TBCrystal *Crystal) {
  return Crystal->SiteCoords;
}

// Unsafe routine. Does not check Crystal for NULL. Acceptable because caller
// verifies.
const double complex *
TBCI_InternalGetTBHamiltonianPtr(const TBCrystal *Crystal) {
  return Crystal->Hamiltonian;
}

// Unsafe routine. Does not check Crystal for NULL. Acceptable because caller
// verifies.
const double complex *TBCI_InternalGetPositionOpPtr(const TBCrystal *Crystal) {
  return Crystal->PositionOp;
}

void FreeTBCrystal(TBCrystal **CrystalReference) {
  if (!CrystalReference) {
    return;
  }
  if (*CrystalReference) {
    free((*CrystalReference)->PositionOp);
    FreeMAC(&((*CrystalReference)->PositionOpSpec));
    free((*CrystalReference)->Hamiltonian);
    FreeMAC(&((*CrystalReference)->HamiltonianSpec));
    free((*CrystalReference)->SiteCoords);
    FreeMAC(&((*CrystalReference)->SiteCoordsSpec));
    free((*CrystalReference)->DirectLattice);
    FreeMAC(&((*CrystalReference)->DirectLatticeSpec));
    free((*CrystalReference));
    *CrystalReference = NULL;
  }
}

TBCrystal *NewTBCrystal(const size_t Dimensionality,
                        const size_t SiteCount,
                        const size_t BandCount) {
  TBCrystal *Crystal = NULL;
  size_t     Size, i;

  bool AllocatedCrystal           = false;
  bool AllocatedDirectLatticeSpec = false;
  bool AllocatedDirectLattice     = false;
  bool AllocatedSiteCoordsSpec    = false;
  bool AllocatedSiteCoords        = false;
  bool AllocatedHamiltonianSpec   = false;
  bool AllocatedHamiltonian       = false;
  bool AllocatedPositionOpSpec    = false;

  if (Dimensionality == 0) {
    return NULL;
  }
  if (BandCount == 0) {
    return NULL;
  }
  if (SiteCount == 0) {
    return NULL;
  }
  Crystal = (TBCrystal *)Core_Malloc(sizeof(TBCrystal));
  if (!Crystal) {
    goto cleanup;
  }
  AllocatedCrystal = true;

  Crystal->DirectLatticeSpec = NULL;
  Crystal->DirectLattice     = NULL;
  Crystal->SiteCoordsSpec    = NULL;
  Crystal->SiteCoords        = NULL;
  Crystal->HamiltonianSpec   = NULL;
  Crystal->Hamiltonian       = NULL;
  Crystal->PositionOpSpec    = NULL;
  Crystal->PositionOp        = NULL;

  // Direct lattice.
  Crystal->DirectLatticeSpec = (MAC *)NewMAC(
      2, (const size_t[]){Dimensionality, Dimensionality}, COLUMN_MAJOR);
  if (!Crystal->DirectLatticeSpec) {
    goto cleanup;
  }
  AllocatedDirectLatticeSpec = true;
  if (Core_MulOverflow(MACGetElementCount(Crystal->DirectLatticeSpec),
                       sizeof(double), &Size)) {
    goto cleanup;
  }
  Crystal->DirectLattice = (double *)Core_Malloc(Size);
  if (!Crystal->DirectLattice) {
    goto cleanup;
  }
  AllocatedDirectLattice = true;
  for (i = 0; i < MACGetElementCount(Crystal->DirectLatticeSpec); i++) {
    Crystal->DirectLattice[i] = 0;
  }

  // Site coordinates.
  Crystal->SiteCoordsSpec = (MAC *)NewMAC(
      2, (const size_t[]){Dimensionality, SiteCount}, COLUMN_MAJOR);
  if (!Crystal->SiteCoordsSpec) {
    goto cleanup;
  }
  AllocatedSiteCoordsSpec = true;
  if (Core_MulOverflow(MACGetElementCount(Crystal->SiteCoordsSpec),
                       sizeof(long int), &Size)) {
    goto cleanup;
  }
  Crystal->SiteCoords = (long int *)Core_Malloc(Size);
  if (!Crystal->SiteCoords) {
    goto cleanup;
  }
  AllocatedSiteCoords = true;
  for (i = 0; i < MACGetElementCount(Crystal->SiteCoordsSpec); i++) {
    Crystal->SiteCoords[i] = (long int)0;
  }

  // Hamiltonian
  Crystal->HamiltonianSpec = (MAC *)NewMAC(
      3, (const size_t[]){SiteCount, BandCount, BandCount}, COLUMN_MAJOR);
  if (!Crystal->HamiltonianSpec) {
    goto cleanup;
  }
  AllocatedHamiltonianSpec = true;
  if (Core_MulOverflow(MACGetElementCount(Crystal->HamiltonianSpec),
                       sizeof(double complex), &Size)) {
    goto cleanup;
  }
  Crystal->Hamiltonian = (double complex *)Core_Malloc(Size);
  if (!Crystal->Hamiltonian) {
    goto cleanup;
  }
  AllocatedHamiltonian = true;
  for (i = 0; i < MACGetElementCount(Crystal->HamiltonianSpec); i++) {
    Crystal->Hamiltonian[i] = (double complex)0;
  }

  // Berry connection.
  Crystal->PositionOpSpec = (MAC *)NewMAC(
      4, (const size_t[]){SiteCount, BandCount, BandCount, Dimensionality},
      COLUMN_MAJOR);
  if (!Crystal->PositionOpSpec) {
    goto cleanup;
  }
  AllocatedPositionOpSpec = true;
  if (Core_MulOverflow(MACGetElementCount(Crystal->PositionOpSpec),
                       sizeof(double complex), &Size)) {
    goto cleanup;
  }
  Crystal->PositionOp = (double complex *)Core_Malloc(Size);
  if (!Crystal->PositionOp) {
    goto cleanup;
  }
  for (i = 0; i < MACGetElementCount(Crystal->PositionOpSpec); i++) {
    Crystal->PositionOp[i] = (double complex)0;
  }

  // Specifiers.
  Crystal->Dimensionality = Dimensionality;
  Crystal->SiteCount      = SiteCount;
  Crystal->BandCount      = BandCount;

  return Crystal;

cleanup:
  if (AllocatedPositionOpSpec)
    FreeMAC(&(Crystal->PositionOpSpec));
  if (AllocatedHamiltonian)
    free(Crystal->Hamiltonian);
  if (AllocatedHamiltonianSpec)
    FreeMAC(&(Crystal->HamiltonianSpec));
  if (AllocatedSiteCoords)
    free(Crystal->SiteCoords);
  if (AllocatedSiteCoordsSpec)
    FreeMAC(&(Crystal->SiteCoordsSpec));
  if (AllocatedDirectLattice)
    free(Crystal->DirectLattice);
  if (AllocatedDirectLatticeSpec)
    FreeMAC(&(Crystal->DirectLatticeSpec));
  if (AllocatedCrystal)
    free(Crystal);
  return NULL;
}

size_t TBCrystalGetDimensionality(const TBCrystal *Crystal) {
  if (Crystal) {
    return Crystal->Dimensionality;
  }
  return 0;
}

size_t TBCrystalGetSiteCount(const TBCrystal *Crystal) {
  if (Crystal) {
    return Crystal->SiteCount;
  }
  return 0;
}

size_t TBCrystalGetBandCount(const TBCrystal *Crystal) {
  if (Crystal) {
    return Crystal->BandCount;
  }
  return 0;
}

bool TBCrystalSetDirectLatticeVal(TBCrystal   *Crystal,
                                  const size_t VectorId,
                                  const size_t ComponentId,
                                  const double Val) {
  size_t Index;

  if (Crystal) {
    if ((VectorId >= Crystal->Dimensionality) ||
        (ComponentId >= Crystal->Dimensionality)) {
      return false;
    }
    MACMultidimensionalToFlat(Crystal->DirectLatticeSpec, 2,
                              (const size_t[]){ComponentId, VectorId}, &Index);
    Crystal->DirectLattice[Index] = Val;
    return true;
  }
  return false;
}

bool TBCrystalGetDirectLatticeVal(const TBCrystal *Crystal,
                                  const size_t     VectorId,
                                  const size_t     ComponentId,
                                  double          *Val) {
  size_t Index;

  if (Crystal) {
    if (!Val) {
      return false;
    }
    if ((VectorId >= Crystal->Dimensionality) ||
        (ComponentId >= Crystal->Dimensionality)) {
      return false;
    }
    MACMultidimensionalToFlat(Crystal->DirectLatticeSpec, 2,
                              (const size_t[]){ComponentId, VectorId}, &Index);
    *Val = Crystal->DirectLattice[Index];
    return true;
  }
  return false;
}

bool TBCrystalSetSiteVal(TBCrystal     *Crystal,
                         const size_t   SiteId,
                         const size_t   ComponentId,
                         const long int Val) {
  size_t Index;

  if (Crystal) {
    if ((SiteId >= Crystal->SiteCount) ||
        (ComponentId >= Crystal->Dimensionality)) {
      return false;
    }
    MACMultidimensionalToFlat(Crystal->SiteCoordsSpec, 2,
                              (const size_t[]){ComponentId, SiteId}, &Index);
    Crystal->SiteCoords[Index] = Val;
    return true;
  }
  return false;
}

bool TBCrystalGetSiteVal(const TBCrystal *Crystal,
                         const size_t     SiteId,
                         const size_t     ComponentId,
                         long int        *Val) {
  size_t Index;

  if (Crystal) {
    if (!Val) {
      return false;
    }
    if ((SiteId >= Crystal->SiteCount) ||
        (ComponentId >= Crystal->Dimensionality)) {
      return false;
    }
    MACMultidimensionalToFlat(Crystal->SiteCoordsSpec, 2,
                              (const size_t[]){ComponentId, SiteId}, &Index);
    *Val = Crystal->SiteCoords[Index];
    return true;
  }
  return false;
}

bool TBCrystalSetTBHamiltonian(TBCrystal           *Crystal,
                               const size_t         BandId1,
                               const size_t         BandId2,
                               const size_t         SiteId,
                               const double complex Val) {
  size_t Index;

  if (Crystal) {
    if ((BandId1 >= Crystal->BandCount) || (BandId2 >= Crystal->BandCount) ||
        (SiteId >= Crystal->SiteCount)) {
      return false;
    }
    MACMultidimensionalToFlat(Crystal->HamiltonianSpec, 3,
                              (const size_t[]){SiteId, BandId1, BandId2},
                              &Index);
    Crystal->Hamiltonian[Index] = Val;
    return true;
  }
  return false;
}

bool TBCrystalGetTBHamiltonian(const TBCrystal *Crystal,
                               const size_t     BandId1,
                               const size_t     BandId2,
                               const size_t     SiteId,
                               double complex  *Val) {
  size_t Index;

  if (Crystal) {
    if (!Val) {
      return false;
    }
    if ((BandId1 >= Crystal->BandCount) || (BandId2 >= Crystal->BandCount) ||
        (SiteId >= Crystal->SiteCount)) {
      return false;
    }
    MACMultidimensionalToFlat(Crystal->HamiltonianSpec, 3,
                              (const size_t[]){SiteId, BandId1, BandId2},
                              &Index);
    *Val = Crystal->Hamiltonian[Index];
    return true;
  }
  return false;
}

bool TBCrystalSetTBPositionOperator(TBCrystal           *Crystal,
                                    const size_t         BandId1,
                                    const size_t         BandId2,
                                    const size_t         SiteId,
                                    const size_t         ComponentId,
                                    const double complex Val) {
  size_t Index;

  if (Crystal) {
    if ((BandId1 >= Crystal->BandCount) || (BandId2 >= Crystal->BandCount) ||
        (SiteId >= Crystal->SiteCount) ||
        (ComponentId >= Crystal->Dimensionality)) {
      return false;
    }
    MACMultidimensionalToFlat(
        Crystal->PositionOpSpec, 4,
        (const size_t[]){SiteId, BandId1, BandId2, ComponentId}, &Index);
    Crystal->PositionOp[Index] = Val;
    return true;
  }
  return false;
}

bool TBCrystalGetTBPositionOperator(const TBCrystal *Crystal,
                                    const size_t     BandId1,
                                    const size_t     BandId2,
                                    const size_t     SiteId,
                                    const size_t     ComponentId,
                                    double complex  *Val) {
  size_t Index;

  if (Crystal) {
    if (!Val) {
      return false;
    }
    if ((BandId1 >= Crystal->BandCount) || (BandId2 >= Crystal->BandCount) ||
        (SiteId >= Crystal->SiteCount) ||
        (ComponentId >= Crystal->Dimensionality)) {
      return false;
    }
    MACMultidimensionalToFlat(
        Crystal->PositionOpSpec, 4,
        (const size_t[]){SiteId, BandId1, BandId2, ComponentId}, &Index);
    *Val = Crystal->PositionOp[Index];
    return true;
  }
  return false;
}
