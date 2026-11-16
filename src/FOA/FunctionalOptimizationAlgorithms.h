#ifndef FunctionalOptimizationAlgorithms_CH
#define FunctionalOptimizationAlgorithms_CH

#include <stddef.h>

/**
 * @brief Objective function evaluated by the FOA algorithms.
 *
 * @details
 * Maps a point of the search space R^Dimension to a real value. The value is
 * consumed by an algorithm through its sign and through its magnitude.
 *
 * The callback is invoked synchronously, on the calling thread, and only
 * while the algorithm which requires it is running.
 *
 * @param[in] Dimension
 *     Number of components of the evaluation point. It equals the Dimension
 *     of the algorithm which invoked the callback.
 *
 * @param[in] EvaluationPoint
 *     Pointer to Dimension doubles holding the point. The pointer is valid
 *     only inside the callback, and the object it addresses must not be
 *     written to.
 *
 * @param[in] UserContext
 *     Optional caller-defined context, forwarded verbatim from the algorithm
 *     which invoked the callback. May be NULL.
 *
 * @return
 *     Value of the objective function at the specified point.
 *
 * @note
 * - The callback must not be NULL: FOA rejects a call which does not supply
 *   one, without evaluating it.
 * - The value must be deterministic: the algorithms assume that the sign and
 *   the magnitude of the objective at a point do not depend on how many times
 *   the callback has been invoked.
 * - A NaN value marks a point without a defined objective value; the
 *   algorithms stop and report the absence of an answer.
 * - FOA keeps no global state. Concurrent calls are safe provided the
 *   callback is thread-safe.
 */
typedef double (*ObjectiveFunctionInterface)(
    const size_t Dimension,
    const double EvaluationPoint[Dimension],
    void        *UserContext);

/**
 * @brief Locates a sign change of an objective function on the segment which
 * joins two pivots.
 *
 * @details
 * -------------------------------------------------------------------------
 * MATHEMATICAL PROBLEM
 * -------------------------------------------------------------------------
 *
 * For an objective f: R^Dimension -> R and two pivots of R^Dimension, the
 * routine searches the segment which joins them,
 *
 *     x(t) = LeftPivot + t * (RightPivot - LeftPivot),   t in [0, 1],
 *
 * and returns the point of the segment at which f changes sign.
 *
 * The only assumption on f is that the two pivots lie on opposite sides of
 * zero. Neither continuity nor differentiability is required, and the pivots
 * may be supplied in either order: the segment is the same.
 *
 * -------------------------------------------------------------------------
 * NUMERICAL METHOD
 * -------------------------------------------------------------------------
 *
 * The sign change is located by bisection along the segment. Every iteration
 * evaluates f at the midpoint of the current bracket and retains the half
 * which still holds the sign change, so the bracket is halved at each step.
 *
 * The objective is evaluated once at each pivot and at most once per
 * iteration: the iteration which finds that the bracket cannot be split
 * returns before evaluating. An iteration costs O(Dimension) floating-point
 * operations with a small constant, and it allocates no memory on the heap.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * In exact arithmetic, after k halvings of the bracket the returned point
 * lies within
 *
 *     |RightPivot - LeftPivot| / 2^k
 *
 * of a sign change, for any norm of R^Dimension: every bracket is a copy of
 * the segment scaled by a power of two.
 *
 * The search stops at whichever of the following conditions is met first: a
 * pivot at which the objective vanishes, a value which is NaN, a residual
 * |f(Root)| less than or equal to Tolerance, a bracket which cannot be halved
 * any further, or MaxIterations iterations. Root is the last estimate in
 * every case, and rounding is the only reason the bound above may not be
 * reached.
 *
 * In double precision, 53 halvings take a segment of unit width down to the
 * spacing of the type, so a larger cap only guards against a bracket which
 * refuses to shrink. A smaller cap is a deliberate trade of accuracy for
 * work.
 *
 * The routine reports no iteration count and no convergence flag: the
 * tolerance is a stopping rule, not a promise, so the caller validates Root.
 *
 * -------------------------------------------------------------------------
 * NOTES
 * -------------------------------------------------------------------------
 *
 * Root holds an answer when the objective takes opposite signs at the pivots,
 * or vanishes at one of them. When it does neither, or when an argument is
 * rejected, Root is filled with NaN.
 *
 * @param[in] Dimension
 *     Number of components of the pivots and of Root. Dimension = 0 is a
 *     valid no-op: no callback is invoked and nothing is written, not even
 *     the NaN fill.
 *
 * @param[in] LeftPivot
 *     Pointer to the Dimension doubles of the first end of the segment. Must
 *     not be NULL.
 *
 * @param[in] RightPivot
 *     Pointer to the Dimension doubles of the second end of the segment.
 *     Must not be NULL.
 *
 * @param[in] ObjectiveFunction
 *     Objective whose sign change is sought. Must not be NULL.
 *
 * @param[in] MaxIterations
 *     Upper bound on the number of bisection iterations. 0 is a rejection:
 *     the routine returns without evaluating the objective.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the residual |f(Root)|, checked at every midpoint
 *     of the bracket. The value only decides when to stop; it is not a bound
 *     on the distance between Root and a zero of f. A negative or NaN value
 *     is a rejection.
 *
 * @param[out] Root
 *     Pointer to the Dimension doubles which receive the result. The buffer
 *     is filled with NaN before any other argument is validated, so every
 *     rejection leaves it with a defined, "no answer" value; the fill is the
 *     only thing a call with Dimension = 0 does not do. Must not be NULL, and
 *     must not overlap either pivot.
 *
 * @param[in] UserContext
 *     Optional caller-defined context, forwarded verbatim to every
 *     invocation of ObjectiveFunction. May be NULL.
 *
 * @return
 *     None. The outcome is reported through Root.
 *
 * @note
 * - The pivots are copied before the first evaluation, so an objective which
 *   reuses the buffers of the caller through UserContext cannot alter the
 *   search.
 * - Root must not overlap either pivot: the NaN fill runs before the pivots
 *   are read, so an overlapping buffer would be overwritten and the call
 *   would report the absence of an answer. For a Fortran caller the question
 *   does not arise, because the result is a new array.
 * - An objective which returns NaN terminates the search: Root is NaN.
 * - The values at the pivots are compared through their sign, never through
 *   their product, so brackets whose product would over- or underflow are
 *   accepted.
 * - A zero at a pivot is an exact answer and is returned unchanged.
 * - FOA keeps no global state. Concurrent calls are safe provided
 *   ObjectiveFunction is thread-safe.
 *
 * @see ObjectiveFunctionInterface
 */
void BisectionRootFinding(const size_t Dimension,
                          const double LeftPivot[Dimension],
                          const double RightPivot[Dimension],
                          const ObjectiveFunctionInterface ObjectiveFunction,
                          const size_t                     MaxIterations,
                          const double                     Tolerance,
                          double                           Root[Dimension],
                          void                            *UserContext);

#endif
