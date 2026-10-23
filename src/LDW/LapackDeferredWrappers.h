#ifndef LapackDeferredWrappers_CH
#define LapackDeferredWrappers_CH

#include <complex.h>
#include <stddef.h>

/**
 * @brief Computes the minimum of two values.
 *
 * @details
 * Evaluates the arguments (potentially multiple times).
 *
 * @param[in] a
 *     The first value of type size_t to compare.
 *
 * @param[in] b
 *     The second value of type size_t to compare.
 *
 * @return
 *     The smaller value between a and b as a size_t.
 *
 * @note
 * Avoid passing expressions with side effects (e.g., x++).
 */
#define LDW_MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief Opaque address of a LAPACK ZHEEVD implementation.
 *
 * @details
 * LDW does not link against LAPACK. The caller supplies the address of a
 * Fortran procedure implementing the LAPACK ZHEEVD ABI, and the final
 * application links the chosen LAPACK implementation.
 *
 * @note
 * - The caller must review the ABI contract of the supplied LAPACK
 * implementation.
 * - The interface assumes IEEE-754 binary64 DOUBLE PRECISION and
 *   COMPLEX*16. Double maps to REAL64 and double complex to
 *   complex(REAL64). It also assumes 32-bit INTEGER, as in reference
 *   LAPACK and the usual LP64 builds.
 * - The supplied procedure must be thread-safe if concurrent calls are
 *   intended; LDW itself keeps no global state.
 */
typedef void (*ZHEEVDSymbol)(void);

/**
 * @brief Computes the eigenvalues and eigenvectors of a complex Hermitian
 * matrix. The LAPACK ZHEEVD implementation is supplied by the caller at run
 * time.
 *
 * @details
 * For a complex Hermitian matrix A of order N,
 *
 *     A = A^H,
 *
 * the routine computes
 *
 *     U^H * A * U = diag(W),   U^H * U = I,
 *
 * where W contains the eigenvalues in ascending order and the columns
 * of U are the corresponding orthonormal eigenvectors.
 *
 * Storage is column-major:
 *
 *     Matrix(i, j) = Matrix[(i-1) + Dimension * (j-1)].
 *
 * The working set is O(Dimension^2).
 *
 * @param[in] Dimension
 *     Order N of the matrix. N >= 0. Values greater than INT32_MAX are
 *     rejected; Dimension = 0 is a valid no-op.
 *
 * @param[in] Matrix
 *     Input N-by-N complex Hermitian matrix in column-major order. Matrix is
 *     not modified. Must not overlap Unitary, Eigenvalues, or Info.
 *
 * @param[out] Unitary
 *     Output N-by-N unitary matrix in column-major order. Column j is the
 *     orthonormal eigenvector associated with Eigenvalues[j]. The array is
 *     overwritten.
 *
 * @param[out] Eigenvalues
 *     Optional output array of N eigenvalues in ascending order. May be
 *     NULL, in which case the eigenvalues are not returned.
 *
 * @param[in] ZHEEVDProcedure
 *     Address of a Fortran LAPACK ZHEEVD implementation. See
 *     ZHEEVDSymbol. Must not be NULL.
 *
 * @param[out] Info
 *     LAPACK's Status code. 0 on success. A nonzero status from the workspace
 *     query or from the computational call is propagated unchanged. If
 *     Dimension > INT32_MAX, Info is set to INT32_MAX. If Dimension == 0, Info
 *     is 0.
 *
 * @note
 * - All pointer arguments are required and must be non-NULL, except
 *   Eigenvalues, which may be NULL.
 * - Matrix, Unitary, Eigenvalues, and Info must not overlap.
 * - The caller is responsible for the ABI contract of the supplied
 *   symbol: Fortran symbol and calling convention.
 * - LDW has no global state. Concurrent calls are safe provided the
 *   supplied ZHEEVD implementation is thread-safe.
 *
 * @see ZHEEVDSymbol
 */
void DiagHermitian(const size_t         Dimension,
                   const double complex Matrix[Dimension * Dimension],
                   double complex       Unitary[Dimension * Dimension],
                   double               Eigenvalues[Dimension],
                   const ZHEEVDSymbol   ZHEEVDProcedure,
                   int                 *Info);

/**
 * @brief Opaque address of a LAPACK ZGEES implementation.
 *
 * @details
 * LDW does not link against LAPACK. The caller supplies the address of a
 * Fortran procedure implementing the LAPACK ZGEES ABI, and the final
 * application links the chosen LAPACK implementation.
 *
 * @note
 * - The caller must review the ABI contract of the supplied LAPACK
 * implementation.
 * - The interface assumes IEEE-754 binary64 DOUBLE PRECISION and
 *   COMPLEX*16. Double maps to REAL64 and double complex to
 *   complex(REAL64). It also assumes 32-bit INTEGER, and 32-bit LOGICAL as in
 * reference LAPACK and the usual LP64 builds.
 * - The supplied procedure must be thread-safe if concurrent calls are
 *   intended; LDW itself keeps no global state.
 */
typedef void (*ZGEESSymbol)(void);

/**
 * @brief Computes the Schur form and eigenvectors of a complex
 * matrix. The LAPACK ZGEES implementation is supplied by the caller at run
 * time.
 *
 * @details
 * For a complex matrix A of order N, the routine computes
 *
 *     U^H * A * U = S,   U^H * U = I,
 *
 * where S is the upper triangular Schur form, and the columns
 * of U are the corresponding orthonormal eigenvectors.
 *
 * Storage is column-major:
 *
 *     Matrix(i, j) = Matrix[(i-1) + Dimension * (j-1)].
 *
 * The working set is O(Dimension^2).
 *
 * @param[in] Dimension
 *     Order N of the matrix. N >= 0. Values greater than INT32_MAX are
 *     rejected; Dimension = 0 is a valid no-op.
 *
 * @param[in] Matrix
 *     Input N-by-N complex matrix in column-major order. Matrix is
 *     not modified. Must not overlap Unitary, SchurForm, or Info.
 *
 * @param[out] Unitary
 *     Output N-by-N unitary matrix in column-major order. Column j is the
 *     orthonormal eigenvector associated with SchurForm(j, j). The array is
 *     overwritten.
 *
 * @param[out] SchurForm
 *     Optional output N-by-N upper triangular matrix in column-major order.
 * Contains the Schur form of Matrix. The array is overwritten. May be NULL,
 * in which case the Schur form is not returned.
 *
 * @param[in] ZGEESProcedure
 *     Address of a Fortran LAPACK ZGEES implementation. See
 *     ZGEESymbol. Must not be NULL.
 *
 * @param[out] Info
 *     LAPACK's Status code. 0 on success. A nonzero status from the workspace
 *     query or from the computational call is propagated unchanged. If
 *     Dimension > INT32_MAX, Info is set to INT32_MAX. If Dimension == 0, Info
 *     is 0.
 *
 * @note
 * - All pointer arguments are required and must be non-NULL, except
 *   SchurForm, which may be NULL.
 * - Matrix, Unitary, SchurForm, and Info must not overlap.
 * - The caller is responsible for the ABI contract of the supplied
 *   symbol: Fortran symbol and calling convention.
 * - LDW has no global state. Concurrent calls are safe provided the
 *   supplied ZGEES implementation is thread-safe.
 *
 * @see ZGEESymbol
 */
void SchurDecomposition(const size_t         Dimension,
                        const double complex Matrix[Dimension * Dimension],
                        double complex       Unitary[Dimension * Dimension],
                        double complex       SchurForm[Dimension * Dimension],
                        const ZGEESSymbol    ZGEESProcedure,
                        int                 *Info);

/**
 * @brief Opaque address of a LAPACK ZGESDD implementation.
 *
 * @details
 * LDW does not link against LAPACK. The caller supplies the address of a
 * Fortran procedure implementing the LAPACK ZGESDD ABI, and the final
 * application links the chosen LAPACK implementation.
 *
 * @note
 * - The caller must review the ABI contract of the supplied LAPACK
 * implementation.
 * - The interface assumes IEEE-754 binary64 DOUBLE PRECISION and
 *   COMPLEX*16. Double maps to REAL64 and double complex to
 *   complex(REAL64). It also assumes 32-bit INTEGER as in
 * reference LAPACK and the usual LP64 builds.
 * - The supplied procedure must be thread-safe if concurrent calls are
 *   intended; LDW itself keeps no global state.
 */
typedef void (*ZGESDDSymbol)(void);

/**
 * @brief Computes the singular value decomposition of a complex matrix.
 * The LAPACK ZGESDD implementation is supplied by the caller at run
 * time.
 *
 * @details
 * For a complex DimensionR-by-DimensionC matrix A, the routine computes
 *
 *     U^H * A * V = diag(S),   U^H * U = I, V^H * V = I,
 *
 * where S contains the singular values in ascending order, and the columns
 * of U and V are the corresponding orthonormal left and right singular
 * vectors. For j = 1 .. min(DimensionR, DimensionC), DiagSigma[j-1] pairs
 * with column j of UnitaryU and row j of UnitaryVH; the remaining columns
 * of U and V complete the orthonormal bases.
 *
 * Storage is column-major:
 *
 *     Matrix(i, j) = Matrix[(i-1) + DimensionR * (j-1)].
 *
 * The working set is O(DimensionR * DimensionC).
 *
 * @param[in] DimensionR
 *     Number of rows of the matrix. DimensionR >= 0. Values greater than
 *     INT32_MAX are rejected; DimensionR = 0 is a valid no-op.
 *
 * @param[in] DimensionC
 *     Number of columns of the matrix. DimensionC >= 0. Values greater than
 *     INT32_MAX are rejected; DimensionC = 0 is a valid no-op.
 *
 * @param[in] Matrix
 *     Input DimensionR-by-DimensionC complex matrix in column-major order.
 *     Matrix is not modified. Must not overlap UnitaryU, DiagSigma, UnitaryVH,
 *     or Info.
 *
 * @param[out] UnitaryU
 *     Output DimensionR-by-DimensionR unitary matrix in column-major order.
 *     For j = 1 .. min(DimensionR, DimensionC), column j is the orthonormal
 *     left singular vector associated with DiagSigma[j-1]; the remaining
 *     columns complete the orthonormal basis. The array is overwritten.
 *
 * @param[out] DiagSigma
 *     Optional output array of min(DimensionR, DimensionC) singular values in
 *     ascending order. May be NULL, in which case the singular values are not
 *     returned.
 *
 * @param[out] UnitaryVH
 *     Output DimensionC-by-DimensionC unitary matrix in column-major order.
 *     For j = 1 .. min(DimensionR, DimensionC), row j is the
 *     complex-conjugated orthonormal right singular vector associated with
 *     DiagSigma[j-1]; the remaining rows complete the orthonormal basis. The
 *     array is overwritten.
 *
 * @param[in] ZGESDDProcedure
 *     Address of a Fortran LAPACK ZGESDD implementation. See
 *     ZGESDDSymbol. Must not be NULL.
 *
 * @param[out] Info
 *     LAPACK's Status code. 0 on success. A nonzero status from the workspace
 *     query or from the computational call is propagated unchanged. If either
 *     dimension exceeds INT32_MAX, Info is set to INT32_MAX. If either
 *     dimension is 0, Info is 0.
 *
 * @note
 * - All pointer arguments are required and must be non-NULL, except
 *   DiagSigma, which may be NULL.
 * - Matrix, UnitaryU, DiagSigma, UnitaryVH, and Info must not overlap.
 * - The caller is responsible for the ABI contract of the supplied
 *   symbol: Fortran symbol and calling convention.
 * - This routine returns conjugate(transpose(V)) = V^(-1), not V.
 * - LDW has no global state. Concurrent calls are safe provided the
 *   supplied ZGESDD implementation is thread-safe.
 *
 * @see ZGESDDSymbol
 */
void SVDDecomposition(const size_t         DimensionR,
                      const size_t         DimensionC,
                      const double complex Matrix[DimensionR * DimensionC],
                      double complex       UnitaryU[DimensionR * DimensionR],
                      double               DiagSigma[LDW_MIN(DimensionR,
                                                             DimensionC)],
                      double complex       UnitaryVH[DimensionC * DimensionC],
                      const ZGESDDSymbol   ZGESDDProcedure,
                      int                 *Info);

/**
 * @brief Identifies degenerate levels in an ascending list of eigenvalues.
 * Consecutive levels belong to the same group if their difference is
 * less than or equal to DegeneracyThreshold.
 *
 * @details
 *     Degeneracy propagates transitively along consecutive levels.
 * If Degeneracies[i] = N, then Eigenvalues[i:i+N-1] forms a degenerate group.
 * The first element of each group stores the group size;
 * the remaining ones are zero.
 *
 * @param[in] Dimension
 *     Order N of the list. N >= 0. Dimension = 0 is a valid no-op.
 *
 * @param[in] Eigenvalues
 *     Input list of eigenvalues. Eigenvalues is
 * not modified. Must not overlap Degeneracies.
 *
 * @param[in] DegeneracyThreshold
 *     Input degeneracy threshold. DegeneracyThreshold <= 0 is a no-op.
 *
 * @param[out] Degeneracies
 *     Output list of degeneracies. Must not overlap Eigenvalues.
 *
 * @note
 * - All pointer arguments are required and must be non-NULL. If any is
 *   NULL, the function returns immediately.
 */
void DegeneracyGroup(const size_t Dimension,
                     const double Eigenvalues[Dimension],
                     const double DegeneracyThreshold,
                     size_t       Degeneracies[Dimension]);

#endif
