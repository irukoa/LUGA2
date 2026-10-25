#ifndef TightBindingCrystalInterpolation_CH
#define TightBindingCrystalInterpolation_CH

#include <complex.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Opaque tight-binding crystal.
 *
 * @details
 * Stores the metadata required to represent a tight-binding crystal:
 *
 * - the direct lattice vectors a_l, in length units,
 * - the integer site offsets n_S, in lattice units,
 * - the lattice-resolved Hamiltonian H_{b1,b2}(S), in energy units, and
 * - the lattice-resolved Berry connection A_{b1,b2,c}(S), in length units,
 *
 * where b1 and b2 in [0, BandCount - 1] are band indexes, c in
 * [0, Dimensionality - 1] is a Cartesian component and S in
 * [0, SiteCount - 1] is a lattice translation. The Cartesian position of
 * translation S is
 *
 *     R_S = sum_{l = 0}^{D - 1} a_l * n_S[l].
 *
 * STORAGE LAYOUT
 * -------------------------------------------------------------------------
 *
 * H_{b1,b2}(S) is stored contiguously as [S][b1][b2] and
 * A_{b1,b2,c}(S) as [S][b1][b2][c], both in COLUMN_MAJOR order: the site
 * indexes run fastest. The accessor functions take
 * the indexes in argument order, not in storage order.
 *
 * Every array is zero-initialized at construction.
 *
 * @note
 * - The internal structure definition is intentionally hidden from users.
 * - Instances are managed exclusively through the public API.
 * - The object owns all internally allocated data.
 * - A physical model requires H_{b1,b2}(S) = conj(H_{b2,b1}(S')) whenever
 * R_{S'} = -R_S.
 *
 * @see NewTBCrystal
 * @see FreeTBCrystal
 */
typedef struct TBCrystal TBCrystal;

/**
 * @brief Frees a tight-binding crystal.
 *
 * @param[in,out] CrystalReference
 *     Reference to the TBCrystal pointer to be freed.
 *
 * @details
 * Releases the direct lattice, the site offsets, the Hamiltonian and the
 * Berry connection storage owned by the specified instance. After the call,
 * the containing pointer is set to NULL.
 *
 * @note
 * - Safe to call with NULL.
 * - Safe to call with a pointer containing NULL.
 * - After successful destruction, the pointed object becomes invalid.
 * - The caller must not access the object after this call.
 *
 * @see NewTBCrystal
 */
void FreeTBCrystal(TBCrystal **CrystalReference);

/**
 * @brief Allocates and initializes a tight-binding crystal.
 *
 * @details
 * Creates a new TBCrystal describing a D-dimensional tight-binding model with
 * SiteCount lattice translations and BandCount bands. Every stored value
 * starts at zero, so the caller must populate the direct lattice, the site
 * offsets, the Hamiltonian blocks and the Berry connection blocks before
 * evaluating k-resolved quantities (see TBCrystalHamiltonian() and
 * TBPositionOperator()).
 *
 * @param[in] Dimensionality
 *     Number of spatial dimensions D. Must be greater than zero.
 *
 * @param[in] SiteCount
 *     Number of lattice translations (hopping shells). Must be greater than
 *     zero.
 *
 * @param[in] BandCount
 *     Number of bands (orbitals) B per site. Must be greater than zero.
 *
 * @return
 * Pointer to a newly allocated TBCrystal on success, or NULL on failure.
 *
 * @note
 * - The returned object is owned by the caller.
 *
 * @see FreeTBCrystal
 * @see TBCrystalSetDirectLatticeVal
 * @see TBCrystalSetSiteVal
 * @see TBCrystalSetTBHamiltonian
 * @see TBCrystalSetTBPositionOperator
 */
TBCrystal *NewTBCrystal(const size_t Dimensionality,
                        const size_t SiteCount,
                        const size_t BandCount);

/**
 * @brief Returns the number of spatial dimensions of a crystal.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @return
 * Dimensionality supplied to NewTBCrystal(), or 0 if Crystal is NULL.
 */
size_t TBCrystalGetDimensionality(const TBCrystal *Crystal);

/**
 * @brief Returns the number of lattice sites of a crystal.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @return
 * SiteCount supplied to NewTBCrystal(), or 0 if Crystal is NULL.
 */
size_t TBCrystalGetSiteCount(const TBCrystal *Crystal);

/**
 * @brief Returns the number of bands of a crystal.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @return
 * BandCount supplied to NewTBCrystal(), or 0 if Crystal is NULL.
 */
size_t TBCrystalGetBandCount(const TBCrystal *Crystal);

/**
 * @brief Sets one Cartesian component of one direct lattice vector.
 *
 * @details
 * Stores Val in DirectLattice(ComponentId, VectorId), that is, in the
 * ComponentId-th Cartesian component of the VectorId-th lattice vector.
 *
 * @param[in,out] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] VectorId
 *     Lattice vector index j, in [0, Dimensionality - 1].
 *
 * @param[in] ComponentId
 *     Cartesian component index c, in [0, Dimensionality - 1].
 *
 * @param[in] Val
 *     Value of the component, in length units.
 *
 * @retval true
 *     The component was stored.
 *
 * @retval false
 *     Crystal is NULL, or one of the indexes is out of range.
 *
 * @see TBCrystalGetDirectLatticeVal
 */
bool TBCrystalSetDirectLatticeVal(TBCrystal   *Crystal,
                                  const size_t VectorId,
                                  const size_t ComponentId,
                                  const double Val);

/**
 * @brief Retrieves one Cartesian component of one direct lattice vector.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] VectorId
 *     Lattice vector index j, in [0, Dimensionality - 1].
 *
 * @param[in] ComponentId
 *     Cartesian component index c, in [0, Dimensionality - 1].
 *
 * @param[out] Val
 *     Pointer receiving the component value, in length units.
 *
 * @retval true
 *     The component was retrieved.
 *
 * @retval false
 *     Crystal is NULL, Val is NULL, or one of the indexes is out of range.
 *     Val is not written on failure.
 *
 * @see TBCrystalSetDirectLatticeVal
 */
bool TBCrystalGetDirectLatticeVal(const TBCrystal *Crystal,
                                  const size_t     VectorId,
                                  const size_t     ComponentId,
                                  double          *Val);

/**
 * @brief Sets one lattice offset component of one site.
 *
 * @details
 * Stores the integer Val in SiteCoords(ComponentId, SiteId). Site offsets are
 * expressed in units of the direct lattice vectors: site S sits at
 *
 *     R_S = sum_{j = 0}^{D - 1} SiteCoords(j, S) * a_j.
 *
 * @param[in,out] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] SiteId
 *     Site index S, in [0, SiteCount - 1].
 *
 * @param[in] ComponentId
 *     Lattice vector index j, in [0, Dimensionality - 1].
 *
 * @param[in] Val
 *     Signed integer offset along a_j.
 *
 * @retval true
 *     The component was stored.
 *
 * @retval false
 *     Crystal is NULL, or one of the indexes is out of range.
 *
 * @see TBCrystalGetSiteVal
 */
bool TBCrystalSetSiteVal(TBCrystal     *Crystal,
                         const size_t   SiteId,
                         const size_t   ComponentId,
                         const long int Val);

/**
 * @brief Retrieves one lattice offset component of one site.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] SiteId
 *     Site index S, in [0, SiteCount - 1].
 *
 * @param[in] ComponentId
 *     Lattice vector index j, in [0, Dimensionality - 1].
 *
 * @param[out] Val
 *     Pointer receiving the signed integer offset along a_j.
 *
 * @retval true
 *     The component was retrieved.
 *
 * @retval false
 *     Crystal is NULL, Val is NULL, or one of the indexes is out of range.
 *     Val is not written on failure.
 *
 * @see TBCrystalSetSiteVal
 */
bool TBCrystalGetSiteVal(const TBCrystal *Crystal,
                         const size_t     SiteId,
                         const size_t     ComponentId,
                         long int        *Val);

/**
 * @brief Sets one band-pair entry of one Hamiltonian block.
 *
 * @details
 * Stores Val in H_{b1,b2}(S), that is, in the (BandId1, BandId2) entry of the
 * B x B block attached to site SiteId. See TBCrystal for the storage layout.
 *
 * @param[in,out] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] BandId1
 *     Row band index b1, in [0, BandCount - 1].
 *
 * @param[in] BandId2
 *     Column band index b2, in [0, BandCount - 1].
 *
 * @param[in] SiteId
 *     Site index S, in [0, SiteCount - 1].
 *
 * @param[in] Val
 *     Complex matrix element, in energy units.
 *
 * @retval true
 *     The entry was stored.
 *
 * @retval false
 *     Crystal is NULL, or one of the indexes is out of range.
 *
 * @note
 * - The block is not transposed.
 * - Hermiticity is not enforced. A physical model requires
 *   H_{b1,b2}(S) = conj(H_{b2,b1}(S')) whenever R_{S'} = -R_S.
 *
 * @see TBCrystalGetTBHamiltonian
 * @see TBCrystalHamiltonian
 */
bool TBCrystalSetTBHamiltonian(TBCrystal           *Crystal,
                               const size_t         BandId1,
                               const size_t         BandId2,
                               const size_t         SiteId,
                               const double complex Val);

/**
 * @brief Retrieves one band-pair entry of one Hamiltonian block.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] BandId1
 *     Row band index b1, in [0, BandCount - 1].
 *
 * @param[in] BandId2
 *     Column band index b2, in [0, BandCount - 1].
 *
 * @param[in] SiteId
 *     Site index S, in [0, SiteCount - 1].
 *
 * @param[out] Val
 *     Pointer receiving the complex matrix element, in energy units.
 *
 * @retval true
 *     The entry was retrieved.
 *
 * @retval false
 *     Crystal is NULL, Val is NULL, or one of the indexes is out of range.
 *     Val is not written on failure.
 *
 * @see TBCrystalSetTBHamiltonian
 */
bool TBCrystalGetTBHamiltonian(const TBCrystal *Crystal,
                               const size_t     BandId1,
                               const size_t     BandId2,
                               const size_t     SiteId,
                               double complex  *Val);

/**
 * @brief Sets one band-pair, Cartesian-component entry of one Berry
 * connection block.
 *
 * @details
 * Stores Val in A_{b1,b2,c}(S), that is, in the (BandId1, BandId2) entry of
 * the ComponentId-th Cartesian block attached to site SiteId. See TBCrystal
 * for the storage layout.
 *
 * @param[in,out] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] BandId1
 *     Row band index b1, in [0, BandCount - 1].
 *
 * @param[in] BandId2
 *     Column band index b2, in [0, BandCount - 1].
 *
 * @param[in] SiteId
 *     Site index S, in [0, SiteCount - 1].
 *
 * @param[in] ComponentId
 *     Cartesian component index c, in [0, Dimensionality - 1].
 *
 * @param[in] Val
 *     Complex matrix element, in length units.
 *
 * @retval true
 *     The entry was stored.
 *
 * @retval false
 *     Crystal is NULL, or one of the indexes is out of range.
 *
 * @note
 * - As for the Hamiltonian blocks, no symmetry is enforced.
 *
 * @see TBCrystalGetTBPositionOperator
 * @see TBPositionOperator
 */
bool TBCrystalSetTBPositionOperator(TBCrystal           *Crystal,
                                    const size_t         BandId1,
                                    const size_t         BandId2,
                                    const size_t         SiteId,
                                    const size_t         ComponentId,
                                    const double complex Val);

/**
 * @brief Retrieves one band-pair, Cartesian-component entry of one Berry
 * connection block.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] BandId1
 *     Row band index b1, in [0, BandCount - 1].
 *
 * @param[in] BandId2
 *     Column band index b2, in [0, BandCount - 1].
 *
 * @param[in] SiteId
 *     Site index S, in [0, SiteCount - 1].
 *
 * @param[in] ComponentId
 *     Cartesian component index c, in [0, Dimensionality - 1].
 *
 * @param[out] Val
 *     Pointer receiving the complex matrix element, in length units.
 *
 * @retval true
 *     The entry was retrieved.
 *
 * @retval false
 *     Crystal is NULL, Val is NULL, or one of the indexes is out of range.
 *     Val is not written on failure.
 *
 * @see TBCrystalSetTBPositionOperator
 */
bool TBCrystalGetTBPositionOperator(const TBCrystal *Crystal,
                                    const size_t     BandId1,
                                    const size_t     BandId2,
                                    const size_t     SiteId,
                                    const size_t     ComponentId,
                                    double complex  *Val);

/**
 * @brief Evaluates the Derivative-th Cartesian k-derivative of the Bloch
 * Hamiltonian at a reciprocal-space point.
 *
 * @details
 * The routine evaluates:
 *
 *     d^m H(k) / dk^m =
 *     d^m/dk^m sum_S H_{b1,b2}(S) * exp(i * k * R_S),
 *
 * in units of energy * length^m for m = Derivative, where H_{b1,b2}(S) is the
 * stored Hamiltonian block (see TBCrystal). Here
 *
 *     k = sum_{j = 0}^{D - 1} KPoint[j] * b_j,
 *     b_j . a_l = 2 * pi * delta_jl,
 *
 * so KPoint holds the dimensionless coordinates of k in the reciprocal
 * lattice basis. KPoint is conventionally restricted to [-0.5, 0.5] in every
 * direction (reduced zone); any real value is accepted, since the phase is
 * periodic in every KPoint[j] with period 1.
 *
 * Derivative = 0 reduces the expression to H(k) itself.
 *
 * -------------------------------------------------------------------------
 * ARRAY LAYOUT
 * -------------------------------------------------------------------------
 *
 * ArrayShape receives the shape of Array, which is
 *
 *     ArrayShape[0]     = BandCount,
 *     ArrayShape[1]     = BandCount,
 *     ArrayShape[2 + t] = Dimensionality,   t = 0, ..., Derivative - 1,
 *
 * so the result is a (Derivative + 2)-ranked array with
 *
 *     ArraySize = BandCount^2 * Dimensionality^Derivative
 *
 * elements. The layout is COLUMN_MAJOR: the first index is
 * contiguous. The result carries units of energy * length^Derivative.
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] Dimensionality
 *     Number of components in KPoint. Must equal the dimensionality of
 *     Crystal.
 *
 * @param[in] KPoint
 *     Reciprocal-space point in relative coordinates, with Dimensionality
 *     elements.
 *
 * @param[in] Derivative
 *     Derivative order m.
 *
 * @param[in] ArraySize
 *     Number of elements in Array. Must equal
 *     BandCount^2 * Dimensionality^Derivative.
 *
 * @param[out] Array
 *     Caller-owned array receiving the derivative, with ArraySize elements.
 *
 * @param[in] ArrayShapeSize
 *     Number of elements in ArrayShape. Must equal Derivative + 2.
 *
 * @param[out] ArrayShape
 *     Caller-owned array receiving the shape of Array, with ArrayShapeSize
 *     elements.
 *
 * @retval true
 *     Array and ArrayShape were filled.
 *
 * @retval false
 *     The evaluation was rejected. Rejection covers a NULL Crystal, a NULL
 *     KPoint, Array or ArrayShape, a Dimensionality mismatch, an
 *     ArrayShapeSize other than Derivative + 2, an ArraySize other than
 *     BandCount^2 * Dimensionality^Derivative, an overflow of Derivative + 2
 *     and an overflow of BandCount^2 * Dimensionality^Derivative. Array and
 *     ArrayShape are left in an unspecified state.
 *
 * @note
 * - The routine performs no Brillouin-zone folding, no hermitization and no
 *   symmetrization of the supplied blocks.
 * - The cost is O(BandCount^2 * Dimensionality^Derivative * SiteCount).
 * - Working storage is O(SiteCount * (Dimensionality + 2) + Derivative + 2).
 * - The routine keeps no state; concurrent calls are safe.
 */
bool TBCrystalHamiltonian(const TBCrystal *Crystal,
                          const size_t     Dimensionality,
                          const double     KPoint[Dimensionality],
                          const size_t     Derivative,
                          const size_t     ArraySize,
                          double complex   Array[ArraySize],
                          const size_t     ArrayShapeSize,
                          size_t           ArrayShape[ArrayShapeSize]);

/**
 * @brief Evaluates the Derivative-th Cartesian k-derivative of the Bloch
 * Berry connection at a reciprocal-space point.
 *
 * @details
 * The routine evaluates:
 *
 *     d^m A_c(k) / dk^m =
 *     d^m/dk^m sum_S A_{b1,b2,c}(S) * exp(i * k * R_S),
 *
 * in units of length^(m+1) for m = Derivative, where A_{b1,b2,c}(S) is
 * Cartesian component c of the stored Berry connection (see TBCrystal).
 * Here
 *
 *     k = sum_{j = 0}^{D - 1} KPoint[j] * b_j,
 *     b_j . a_l = 2 * pi * delta_jl,
 *
 * so KPoint holds the dimensionless coordinates of k in the reciprocal
 * lattice basis. KPoint is conventionally restricted to [-0.5, 0.5] in every
 * direction (reduced zone); any real value is accepted, since the phase is
 * periodic in every KPoint[j] with period 1.
 *
 * Derivative = 0 reduces the expression to the Berry connection A_c(k)
 * itself.
 *
 * -------------------------------------------------------------------------
 * ARRAY LAYOUT
 * -------------------------------------------------------------------------
 *
 * ArrayShape receives the shape of Array, which is
 *
 *     ArrayShape[0]     = BandCount,
 *     ArrayShape[1]     = BandCount,
 *     ArrayShape[2]     = Dimensionality (cartesian index c),
 *     ArrayShape[3 + t] = Dimensionality,   t = 0, ..., Derivative - 1,
 *
 * so the result is a (Derivative + 3)-ranked array with
 *
 *     ArraySize = BandCount^2 * Dimensionality^(Derivative + 1)
 *
 * elements. The layout is COLUMN_MAJOR: the first index is
 * contiguous. The result carries units of length^(Derivative + 1).
 *
 * @param[in] Crystal
 *     Pointer to the TBCrystal instance.
 *
 * @param[in] Dimensionality
 *     Number of components in KPoint. Must equal the dimensionality of
 *     Crystal.
 *
 * @param[in] KPoint
 *     Reciprocal-space point in relative coordinates, with Dimensionality
 *     elements.
 *
 * @param[in] Derivative
 *     Derivative order m.
 *
 * @param[in] ArraySize
 *     Number of elements in Array. Must equal
 *     BandCount^2 * Dimensionality^(Derivative + 1).
 *
 * @param[out] Array
 *     Caller-owned array receiving the derivative, with ArraySize elements.
 *
 * @param[in] ArrayShapeSize
 *     Number of elements in ArrayShape. Must equal Derivative + 3.
 *
 * @param[out] ArrayShape
 *     Caller-owned array receiving the shape of Array, with ArrayShapeSize
 *     elements.
 *
 * @retval true
 *     Array and ArrayShape were filled.
 *
 * @retval false
 *     The evaluation was rejected. Rejection covers a NULL Crystal, a NULL
 *     KPoint, Array or ArrayShape, a Dimensionality mismatch, an
 *     ArrayShapeSize other than Derivative + 3, an ArraySize other than
 *     BandCount^2 * Dimensionality^(Derivative + 1), an overflow of
 *     Derivative + 3 and an overflow of
 *     BandCount^2 * Dimensionality^(Derivative + 1). Array and ArrayShape are
 *     left in an unspecified state.
 *
 * @note
 * - The routine performs no Brillouin-zone folding, no hermitization and no
 *   symmetrization of the supplied blocks.
 * - The cost is O(BandCount^2 * Dimensionality^(Derivative + 1) * SiteCount).
 * - Working storage is O(SiteCount * (Dimensionality + 2) + Derivative + 3).
 * - The routine keeps no state; concurrent calls are safe.
 */
bool TBPositionOperator(const TBCrystal *Crystal,
                        const size_t     Dimensionality,
                        const double     KPoint[Dimensionality],
                        const size_t     Derivative,
                        const size_t     ArraySize,
                        double complex   Array[ArraySize],
                        const size_t     ArrayShapeSize,
                        size_t           ArrayShape[ArrayShapeSize]);

#endif
