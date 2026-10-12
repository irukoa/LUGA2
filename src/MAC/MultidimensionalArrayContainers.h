#ifndef MultidimensionalArrayContainers_CH
#define MultidimensionalArrayContainers_CH

#include <stddef.h>

/**
 * @brief Memory layout convention for multidimensional array indexing.
 *
 * @details
 * Specifies how a multidimensional index is mapped to a flat linear
 * index and vice versa.
 *
 * Layout interpretation:
 * - COLUMN_MAJOR:
 *   Consecutive elements of the first dimension are contiguous in memory.
 *   This matches the default layout used by the Fortran language.
 *
 * - ROW_MAJOR:
 *   Consecutive elements of the last dimension are contiguous in memory.
 *   This matches the default layout used by the C language.
 *
 * @note
 * The selected layout affects all index conversion operations performed
 * by the module.
 */
typedef enum {
  COLUMN_MAJOR = 1,
  ROW_MAJOR    = 2
} MACLayout;

/**
 * @brief Opaque multidimensional array container (MAC).
 *
 * @details
 * Stores the metadata required to convert between flat linear indexes and
 * multidimensional indexes.
 *
 * A MAC instance internally maintains:
 * - the number of dimensions (rank),
 * - the size of each dimension (shape),
 * - the total number of addressable elements, and
 * - the selected memory layout convention.
 *
 * @note
 * - The internal structure definition is intentionally hidden from users.
 * - Instances are managed exclusively through the public API.
 * - The object owns any internally allocated memory associated with shape
 *   metadata.
 */
typedef struct MAC MAC;

/**
 * @brief Frees a multidimensional array container (MAC).
 *
 * @param[in,out] ContainerReference
 *     Reference to the MAC pointer to be freed.
 *
 * @details
 * Releases all memory owned by the specified MAC instance, including
 * internally allocated shape metadata. After the call, the containing pointer
 * is set to NULL.
 *
 * @note
 * - Safe to call with NULL.
 * - Safe to call with a pointer containing NULL.
 * - After successful destruction, the pointed object becomes invalid.
 * - The caller must not access the object after this call.
 */
void FreeMAC(MAC **ContainerReference);

/**
 * @brief Allocates and initializes a multidimensional array container (MAC).
 *
 * @details
 * Creates a new MAC instance configured for a multidimensional array
 * of the specified rank, shape, and memory layout.
 *
 * @param[in] Rank
 *     Number of dimensions in the array.
 *
 * @param[in] Shape
 *     Array containing the extent of each dimension.
 *     Must contain exactly Rank elements.
 *
 * @param[in] Layout
 *     Memory layout convention used for index conversion.
 *
 * @return
 * Pointer to a newly allocated MAC instance on success, or NULL on
 * failure.
 *
 * @note
 * - The returned object is owned by the caller.
 * - Shape metadata is internally copied and managed by the object.
 * - Rank should be greater than zero.
 */
MAC *NewMAC(const size_t    Rank,
            const size_t    Shape[Rank],
            const MACLayout Layout);

/**
 * @brief Returns the rank of a multidimensional array container (MAC).
 *
 * @param[in] Container
 *     Pointer to the MAC instance.
 *
 * @return
 * Number of dimensions represented by the object.
 *
 * @note
 * - The returned value corresponds to the number of elements in the shape
 *   array.
 */
size_t MACGetRank(const MAC *Container);

/**
 * @brief Returns the shape of a multidimensional array container (MAC).
 *
 * @param[in] Container
 *     Pointer to the MAC instance.
 *
 * @return
 * Pointer to an internal array containing the extent of each dimension.
 *
 * @note
 * - The returned pointer is owned by the MAC instance.
 * - The caller must not modify or free the returned array.
 * - The array contains exactly MACGetRank(Container) elements.
 *
 * @see MACGetRank
 */
const size_t *MACGetShape(const MAC *Container);

/**
 * @brief Returns the total number of addressable elements of a multidimensional
 * array container (MAC).
 *
 * @param[in] Container
 *     Pointer to the MAC instance.
 *
 * @return
 * Total number of elements represented by the multidimensional shape.
 *
 * @details
 * The returned value corresponds to the product of all dimension extents.
 */
size_t MACGetElementCount(const MAC *Container);

/**
 * @brief Returns the memory layout convention used by the multidimensional
 * array container (MAC).
 *
 * @param[in] Container
 *     Pointer to the MAC instance.
 *
 * @return
 * The configured multidimensional memory layout.
 */
MACLayout MACGetLayout(const MAC *Container);

/**
 * @brief Converts a flat linear index into a multidimensional index.
 *
 * @details
 * Computes the multidimensional index corresponding to the specified flat
 * index using the layout convention stored in Container.
 *
 * The resulting index is written into the caller-provided
 * MultidimensionalIndex array.
 *
 * @param[in] Container
 *     Pointer to the MAC instance.
 *
 * @param[in] Rank
 *     Number of dimensions expected in MultidimensionalIndex.
 *
 * @param[in] FlatIndex
 *     Flat linear index to convert.
 *
 * @param[out] MultidimensionalIndex
 *     Caller-owned array receiving the multidimensional index.
 *     Must contain Rank elements.
 *
 * @note
 * - The caller retains ownership of MultidimensionalIndex.
 * - The contents of MultidimensionalIndex are overwritten.
 * - Rank should match the rank stored in MAC.
 * - Behavior is undefined if FlatIndex exceeds the valid element range.
 *
 * @see MACMultidimensionalToFlat
 */
void MACFlatToMultidimensional(const MAC   *Container,
                               const size_t Rank,
                               const size_t FlatIndex,
                               size_t       MultidimensionalIndex[Rank]);

/**
 * @brief Converts a multidimensional index into a flat linear index.
 *
 * @details
 * Computes the flat linear index corresponding to the specified
 * multidimensional index using the layout convention stored in Container.
 *
 * The resulting flat index is written to the caller-provided FlatIndex
 * pointer.
 *
 * @param[in] Container
 *     Pointer to the MAC instance.
 *
 * @param[in] Rank
 *     Number of dimensions represented in MultidimensionalIndex.
 *
 * @param[in] MultidimensionalIndex
 *     Caller-owned array containing a multidimensional index.
 *     Must contain Rank elements.
 *
 * @param[out] FlatIndex
 *     Pointer receiving the computed flat linear index.
 *
 * @note
 * - The caller retains ownership of MultidimensionalIndex.
 * - The contents of FlatIndex are overwritten.
 * - Rank should match the rank stored in MAC.
 * - Behavior is undefined if any index exceeds its corresponding
 *   dimension extent.
 *
 * @see MACFlatToMultidimensional
 */
void MACMultidimensionalToFlat(const MAC   *Container,
                               const size_t Rank,
                               const size_t MultidimensionalIndex[Rank],
                               size_t      *FlatIndex);

#endif
