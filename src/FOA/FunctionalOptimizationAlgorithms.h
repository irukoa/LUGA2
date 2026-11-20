#ifndef FunctionalOptimizationAlgorithms_CH
#define FunctionalOptimizationAlgorithms_CH

#include <stddef.h>

/**
 * @brief Objective function evaluated by the FOA algorithms.
 *
 * @details
 * Maps a point of the search space R^Dimension to a real value, which an
 * algorithm consumes through its sign, through its magnitude, or through both.
 * The callback runs synchronously, on the calling thread, and only while the
 * algorithm which requires it is running.
 *
 * The value must be deterministic: the sign and the magnitude at a point must
 * not depend on how many times the callback has been invoked. A NaN value marks
 * a point without a defined objective value; the algorithms stop and report the
 * absence of an answer.
 *
 * @param[in] Dimension
 *     Number of components of the evaluation point, as passed by the algorithm
 *     which invokes the callback.
 *
 * @param[in] EvaluationPoint
 *     Pointer to Dimension doubles holding the point. It is valid only inside
 *     the callback, and must not be written to.
 *
 * @param[in] UserContext
 *     Caller-defined context, forwarded verbatim from the algorithm which
 *     invokes the callback. May be NULL.
 *
 * @return
 *     Value of the objective function at the specified point.
 *
 * @note
 * - The callback must not be NULL: FOA rejects such a call without evaluating
 *   it.
 * - FOA keeps no global state. Concurrent calls are safe provided the callback
 *   is thread-safe.
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
 * and returns the point of the segment at which f changes sign. The only
 * assumption on f is that the pivots lie on opposite sides of zero: neither
 * continuity nor differentiability is required, and the pivots may be supplied
 * in either order.
 *
 * -------------------------------------------------------------------------
 * NUMERICAL METHOD
 * -------------------------------------------------------------------------
 *
 * Bisection along the segment. Every iteration evaluates f at the midpoint of
 * the current bracket and keeps the half which still holds the sign change. The
 * midpoint is the sum of the halves of the two ends, and the ends are compared
 * through their sign and never through their product: neither the point nor the
 * comparison can over- or underflow.
 *
 * Two points are evaluated before the first iteration and at most one per
 * iteration afterwards.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * In exact arithmetic, after k halvings the returned point lies within
 *
 *     |RightPivot - LeftPivot| / 2^k
 *
 * of a sign change, for any norm of R^Dimension, because every bracket is a
 * copy of the segment scaled by a power of two.
 *
 * The search stops at whichever of these conditions comes first: a pivot at
 * which the objective vanishes, a value which is NaN, a residual |f(Root)| less
 * than or equal to Tolerance, a bracket which cannot be halved any further, or
 * MaxIterations iterations. Root is the last estimate in every case. No flag
 * reports which condition was met, and Tolerance is a stopping rule and not a
 * promise, so the caller validates Root.
 *
 * -------------------------------------------------------------------------
 * NOTES
 * -------------------------------------------------------------------------
 *
 * Tolerance bounds the residual and not the distance between Root and a zero of
 * f. Root holds an answer when the objective takes opposite signs at the pivots
 * or vanishes at one of them; when it does neither, or when an argument is
 * rejected, Root is filled with NaN.
 *
 * @param[in] Dimension
 *     Number of components of the pivots and of Root. 0 is a valid no-op.
 *
 * @param[in] LeftPivot
 *     Pointer to the Dimension doubles of the first pivot. Must not be NULL.
 *
 * @param[in] RightPivot
 *     Pointer to the Dimension doubles of the second pivot. Must not be NULL.
 *
 * @param[in] ObjectiveFunction
 *     Objective whose sign change is sought. Must not be NULL.
 *
 * @param[in] MaxIterations
 *     Upper bound on the number of iterations. 0 is a rejection.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the residual |f(Root)|, checked at every
 *     midpoint. A negative or NaN value is a rejection.
 *
 * @param[out] Root
 *     Pointer to the Dimension doubles which receive the result. Must not be
 *     NULL and must not overlap either pivot.
 *
 * @param[in] UserContext
 *     Caller-defined context, forwarded verbatim. May be NULL.
 *
 * @return
 *     None. The outcome is reported through Root.
 *
 * @note
 * - Root is filled with NaN before any other argument is validated, so every
 *   rejection leaves it with a defined, "no answer" value; a call with
 *   Dimension = 0 does not even fill it.
 * - The result must not overlap either pivot: the fill would overwrite it and
 *   report the absence of an answer. A Fortran caller always gets a new array.
 * - The pivots are copied before the first evaluation, so an objective which
 *   reuses their buffers through UserContext cannot alter the search.
 * - A non-finite pivot (NaN or infinite) is a rejection.
 * - An objective which returns NaN terminates the search: Root is NaN.
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

/**
 * @brief Locates the minimum of an objective function on the segment which
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
 * and returns the point of the segment at which f is smallest. The only
 * assumption on f is that it is unimodal there: nothing checks it, and the
 * pivots may be supplied in either order.
 *
 * -------------------------------------------------------------------------
 * NUMERICAL METHOD
 * -------------------------------------------------------------------------
 *
 * Golden-section search along the segment. The bracket is kept in the parameter
 * t and holds two interior points at
 *
 *     t = hi - rho * (hi - lo)     and     t = lo + rho * (hi - lo),
 *
 * where rho is (sqrt(5) - 1) / 2, near 0.618. The interior point whose value is
 * larger is discarded together with the part of the bracket beyond it, so the
 * bracket keeps a fraction rho of its width per iteration, and the point which
 * is kept is not evaluated again. Two points are evaluated before the first
 * iteration and at most one per iteration afterwards.
 *
 * A point of the segment is the convex combination (1 - t) * LeftPivot +
 * t * RightPivot, which lies between the pivots and never forms their
 * difference, so the points cannot overflow. The length of the segment is
 * measured in halves for the same reason, and is capped at the largest value of
 * the type when the segment is wider than its range.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * In exact arithmetic, after k iterations the bracket keeps a fraction rho^k of
 * the segment, and the returned point is its midpoint, so it lies within
 *
 *     rho^k * |RightPivot - LeftPivot| / 2
 *
 * of a minimizer of f, measured in the largest component. The search stops at
 * whichever of these conditions is met first: a bracket whose width is less
 * than or equal to Tolerance, a value which is NaN, a bracket reduced to two
 * coincident points, or MaxIterations iterations. In the first case the
 * returned point lies within Tolerance / 2 of a minimizer.
 *
 * Tolerance bounds the width of the bracket, and not the value of the
 * objective: it is a stopping rule and not a promise, so the caller validates
 * Minimum.
 *
 * -------------------------------------------------------------------------
 * NOTES
 * -------------------------------------------------------------------------
 *
 * Minimum holds an answer only when the objective is unimodal along the
 * segment. When it is not, the search still returns the local minimum of the
 * last bracket, and reports nothing. The pivots themselves are never evaluated:
 * the two evaluations which precede the first iteration are interior points, so
 * a minimum at a pivot is approached only as the bracket shrinks onto it.
 *
 * @param[in] Dimension
 *     Number of components of the pivots and of Minimum. 0 is a valid no-op.
 *
 * @param[in] LeftPivot
 *     Pointer to the Dimension doubles of the first pivot. Must not be NULL.
 *
 * @param[in] RightPivot
 *     Pointer to the Dimension doubles of the second pivot. Must not be NULL.
 *
 * @param[in] ObjectiveFunction
 *     Objective whose minimum is sought. Must not be NULL.
 *
 * @param[in] MaxIterations
 *     Upper bound on the number of iterations. 0 is a rejection.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the width of the bracket, in the units of the
 *     points. A value which is not smaller than the length of the segment
 *     stops before the first evaluation, and Minimum is the midpoint of the
 *     segment. A negative or NaN value is a rejection.
 *
 * @param[out] Minimum
 *     Pointer to the Dimension doubles which receive the result. Must not be
 *     NULL and must not overlap either pivot.
 *
 * @param[in] UserContext
 *     Caller-defined context, forwarded verbatim. May be NULL.
 *
 * @return
 *     None. The outcome is reported through Minimum.
 *
 * @note
 * - The returned point is the midpoint of the last bracket, an estimate: the
 *   objective need not have been evaluated there, and no exact answer can be
 *   reached, because rho is irrational.
 * - Minimum is filled with NaN before any other argument is validated, so every
 *   rejection leaves it with a defined, "no answer" value; a call with
 *   Dimension = 0 does not even fill it.
 * - The result must not overlap either pivot: the fill would overwrite it and
 *   report the absence of an answer. A Fortran caller always gets a new array.
 * - The memory of the pivots is read at every iteration: an objective which
 *   writes to it through UserContext alters the search.
 * - A non-finite pivot (NaN or infinite) is a rejection.
 * - An objective which returns NaN terminates the search: Minimum is NaN.
 * - FOA keeps no global state. Concurrent calls are safe provided
 *   ObjectiveFunction is thread-safe.
 *
 * @see ObjectiveFunctionInterface
 */
void GoldenSectionMinimization(
    const size_t                     Dimension,
    const double                     LeftPivot[Dimension],
    const double                     RightPivot[Dimension],
    const ObjectiveFunctionInterface ObjectiveFunction,
    const size_t                     MaxIterations,
    const double                     Tolerance,
    double                           Minimum[Dimension],
    void                            *UserContext);

/**
 * @brief Locates a sign change of an objective function on the segment which
 * joins two pivots.
 *
 * @details
 * -------------------------------------------------------------------------
 * MATHEMATICAL PROBLEM
 * -------------------------------------------------------------------------
 *
 * The problem is that of BisectionRootFinding: the segment which joins the two
 * pivots, and the assumption that the objective takes opposite signs at them.
 *
 * -------------------------------------------------------------------------
 * NUMERICAL METHOD
 * -------------------------------------------------------------------------
 *
 * Brent's root finding method along the segment. The bracket is kept in the
 * parameter t, between two estimates which lie on opposite sides of zero, and
 * the two estimates which precede the current one are kept as well. Every
 * iteration forms the next estimate by inverse quadratic interpolation through
 * the three most recent ones, by the secant through the two most recent ones
 * when the first and the third coincide, and by bisection of the bracket
 * whenever the interpolated point falls outside the bracket or is not shorter
 * than the step which produced the current estimate. Interpolation is therefore
 * always backed by bisection: a step is accepted only if it is shorter than
 * half of the step which produced the current estimate, so the bracket is
 * halved at least every second iteration, and much faster when the objective
 * is smooth.
 *
 * A point of the segment is the convex combination (1 - t) * LeftPivot +
 * t * RightPivot, so the points cannot overflow, and the values are compared
 * through their sign, never through their product. Two points are evaluated
 * before the first iteration and at most one per iteration afterwards.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * The search falls back on bisection whenever the interpolation cannot be
 * trusted, so the bracket is halved at least every second iteration, and the
 * search needs at most twice the iterations of BisectionRootFinding. On a
 * smooth objective it needs far fewer, because the estimate converges
 * superlinearly.
 *
 * The search stops at whichever of these conditions is met first: a pivot at
 * which the objective vanishes, a value which is NaN, a residual |f(Root)| less
 * than or equal to Tolerance, a bracket which has reached the spacing of the
 * parameter, or MaxIterations iterations. Root is the last estimate in every
 * case. No flag reports which condition was met, and Tolerance is a stopping
 * rule and not a promise, so the caller validates Root.
 *
 * -------------------------------------------------------------------------
 * NOTES
 * -------------------------------------------------------------------------
 *
 * Root holds an answer when the objective takes opposite signs at the pivots,
 * or vanishes at one of them; when it does neither, or when an argument is
 * rejected, Root is filled with NaN.
 *
 * @param[in] Dimension
 *     Number of components of the pivots and of Root. 0 is a valid no-op.
 *
 * @param[in] LeftPivot
 *     Pointer to the Dimension doubles of the first pivot. Must not be NULL.
 *
 * @param[in] RightPivot
 *     Pointer to the Dimension doubles of the second pivot. Must not be NULL.
 *
 * @param[in] ObjectiveFunction
 *     Objective whose sign change is sought. Must not be NULL.
 *
 * @param[in] MaxIterations
 *     Upper bound on the number of iterations. 0 is a rejection. The worst case
 *     needs twice the iterations of BisectionRootFinding, so a cap which suits
 *     that search may need to be doubled.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the residual |f(Root)|, checked at every
 *     estimate. A negative or NaN value is a rejection.
 *
 * @param[out] Root
 *     Pointer to the Dimension doubles which receive the result. Must not be
 *     NULL and must not overlap either pivot.
 *
 * @param[in] UserContext
 *     Caller-defined context, forwarded verbatim. May be NULL.
 *
 * @return
 *     None. The outcome is reported through Root.
 *
 * @note
 * - Root is an estimate, not an exact answer: it is the last point which the
 *   search evaluated inside a bracket that still held the sign change.
 * - Root is filled with NaN before any other argument is validated, so every
 *   rejection leaves it with a defined, "no answer" value; a call with
 *   Dimension = 0 does not even fill it.
 * - The result must not overlap either pivot: the fill would overwrite it and
 *   report the absence of an answer. A Fortran caller always gets a new array.
 * - The memory of the pivots is read at every iteration: an objective which
 *   writes to it through UserContext alters the search.
 * - A non-finite pivot (NaN or infinite) is a rejection, and so is a pair of
 *   pivots on the same side of zero.
 * - An objective which returns NaN terminates the search: Root is NaN.
 * - The points evaluated by the objective lie on the segment: the interpolation
 *   never extrapolates.
 * - FOA keeps no global state. Concurrent calls are safe provided
 *   ObjectiveFunction is thread-safe.
 *
 * @see ObjectiveFunctionInterface
 */
void BrentRootFinding(const size_t                     Dimension,
                      const double                     LeftPivot[Dimension],
                      const double                     RightPivot[Dimension],
                      const ObjectiveFunctionInterface ObjectiveFunction,
                      const size_t                     MaxIterations,
                      const double                     Tolerance,
                      double                           Root[Dimension],
                      void                            *UserContext);

/**
 * @brief Locates the minimum of an objective function on the segment which
 * joins two pivots.
 *
 * @details
 * -------------------------------------------------------------------------
 * MATHEMATICAL PROBLEM
 * -------------------------------------------------------------------------
 *
 * The problem is that of GoldenSectionMinimization: the segment which joins the
 * two pivots, and the assumption that the objective is unimodal along it.
 *
 * -------------------------------------------------------------------------
 * NUMERICAL METHOD
 * -------------------------------------------------------------------------
 *
 * Brent's method for minimization along the segment. The bracket is kept in the
 * parameter t together with three estimates: x, the best point found, and w and
 * v, the two points which precede it. Every iteration fits the parabola through
 * the three, and evaluates its vertex when the vertex lies inside the bracket
 * and is nearer to x than the step which produced w. Otherwise it takes the
 * golden-section step
 *
 *     d = c * e,     c = (3 - sqrt(5)) / 2, near 0.382,
 *
 * towards the larger part of the bracket. The point whose value is larger is
 * discarded together with the part of the bracket beyond it, so the three
 * estimates keep their meaning: the parabola is replaced by another as soon as
 * the objective departs from it.
 *
 * A point of the segment is the convex combination (1 - t) * LeftPivot +
 * t * RightPivot, so the points cannot overflow. The length of the segment is
 * measured in halves for the same reason, and is capped at the largest value of
 * the type when the segment is wider than its range. One point is evaluated
 * before the first iteration and at most one per iteration afterwards.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * Whenever the parabola is rejected the bracket keeps a fraction rho of its
 * width, with rho = (sqrt(5) - 1) / 2, exactly as in golden-section search.
 * The rejected step is the golden-section step, so the worst case is that of
 * that search, up to a small factor in the iteration count, while a smooth
 * objective converges superlinearly and reaches the tolerance far sooner.
 *
 * The search stops at whichever of these conditions is met first: a bracket
 * whose width is less than or equal to twice Tolerance, a value which is NaN, a
 * step which lands on an end of the bracket or cannot move the estimate, or
 * MaxIterations iterations. In the first case the returned point lies within
 * twice Tolerance of a minimizer, and rounding is the only reason why that
 * bound may not be reached.
 *
 * Tolerance bounds the width of the bracket, and not the value of the
 * objective: it is a stopping rule and not a promise, so the caller validates
 * Minimum. No step lands nearer to the estimate than Tolerance, so the bracket
 * is measured against twice that value.
 *
 * -------------------------------------------------------------------------
 * NOTES
 * -------------------------------------------------------------------------
 *
 * Minimum holds an answer only when the objective is unimodal along the
 * segment. When it is not, it is the best point of the last bracket, and
 * nothing reports it. The pivots themselves are never evaluated, so a minimum
 * at a pivot is approached only as the bracket shrinks onto it.
 *
 * @param[in] Dimension
 *     Number of components of the pivots and of Minimum. 0 is a valid no-op.
 *
 * @param[in] LeftPivot
 *     Pointer to the Dimension doubles of the first pivot. Must not be NULL.
 *
 * @param[in] RightPivot
 *     Pointer to the Dimension doubles of the second pivot. Must not be NULL.
 *
 * @param[in] ObjectiveFunction
 *     Objective whose minimum is sought. Must not be NULL.
 *
 * @param[in] MaxIterations
 *     Upper bound on the number of iterations. 0 is a rejection.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the width of the bracket, in the units of the
 *     points: the search stops once the width is not greater than twice this
 *     value. A value which is not smaller than half the length of the segment
 *     stops before the first evaluation, and Minimum is the midpoint of the
 *     segment. A negative or NaN value is a rejection.
 *
 * @param[out] Minimum
 *     Pointer to the Dimension doubles which receive the result. Must not be
 *     NULL and must not overlap either pivot.
 *
 * @param[in] UserContext
 *     Caller-defined context, forwarded verbatim. May be NULL.
 *
 * @return
 *     None. The outcome is reported through Minimum.
 *
 * @note
 * - The returned point was evaluated by the objective, unless the search
 *   stopped before the first evaluation, in which case it is the midpoint of
 *   the segment.
 * - Minimum is filled with NaN before any other argument is validated, so every
 *   rejection leaves it with a defined, "no answer" value; a call with
 *   Dimension = 0 does not even fill it.
 * - The result must not overlap either pivot: the fill would overwrite it and
 *   report the absence of an answer. A Fortran caller always gets a new array.
 * - The memory of the pivots is read at every iteration: an objective which
 *   writes to it through UserContext alters the search.
 * - A non-finite pivot (NaN or infinite) is a rejection.
 * - An objective which returns NaN terminates the search: Minimum is NaN.
 * - FOA keeps no global state. Concurrent calls are safe provided
 *   ObjectiveFunction is thread-safe.
 *
 * @see ObjectiveFunctionInterface
 */
void BrentMinimization(const size_t                     Dimension,
                       const double                     LeftPivot[Dimension],
                       const double                     RightPivot[Dimension],
                       const ObjectiveFunctionInterface ObjectiveFunction,
                       const size_t                     MaxIterations,
                       const double                     Tolerance,
                       double                           Minimum[Dimension],
                       void                            *UserContext);

#endif
