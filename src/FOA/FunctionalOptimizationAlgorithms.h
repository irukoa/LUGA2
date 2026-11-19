#ifndef FunctionalOptimizationAlgorithms_CH
#define FunctionalOptimizationAlgorithms_CH

#include <stddef.h>

/**
 * @brief Objective function evaluated by the FOA algorithms.
 *
 * @details
 * Maps a point of the search space R^Dimension to a real value. The value is
 * consumed by an algorithm through its sign, through its magnitude, or through
 * both.
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
 * Bisection along the segment. Every iteration evaluates f at the midpoint of
 * the current bracket and retains the half which still holds the sign change,
 * so the bracket is halved at each step. The midpoint is the sum of the halves
 * of the two ends, and the values at the ends are compared through their sign
 * and never through their product: neither the midpoint nor the comparison can
 * over- or underflow.
 *
 * Two points are evaluated before the first iteration and at most one per
 * iteration afterwards: the iteration which finds that the bracket cannot be
 * split returns before evaluating.
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
 * - A pivot which is not finite (NaN or infinite) is a rejection: the
 *   objective is not evaluated, and Root is NaN.
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
 * Golden-section search along the segment. The bracket is kept in the
 * parameter t and holds two interior points at
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
 * In exact arithmetic, after k iterations the bracket keeps a fraction rho^k
 * of the segment, and the returned point is its midpoint, so it lies within
 *
 *     rho^k * |RightPivot - LeftPivot| / 2
 *
 * of a minimizer of f, measured in the largest component. The search stops at
 * whichever of these conditions is met first: a bracket whose width is less
 * than or equal to Tolerance, a value which is NaN, a bracket reduced to two
 * coincident points, or MaxIterations iterations. In the first case the
 * returned point lies within Tolerance / 2 of a minimizer, and rounding is the
 * only reason why it may not be reached.
 *
 * The bracket lives in t, so what rho shrinks is a number of [0, 1] and not the
 * points: in double precision the two interior parameters are adjacent values
 * of the type after some 75 iterations, the interior points then coincide, and
 * the search ends. That count follows from the type and not from the segment,
 * so a MaxIterations above it changes nothing, and a Tolerance below the
 * spacing of the points is replaced by that collapse.
 *
 * Tolerance bounds the width of the bracket in the units of the points, and not
 * the value of the objective: it is a stopping rule, not a promise about the
 * minimum, so the caller validates Minimum. A Tolerance which is not smaller
 * than the length of the segment stops before anything is evaluated, and
 * Minimum is the midpoint of the segment.
 *
 * -------------------------------------------------------------------------
 * NOTES
 * -------------------------------------------------------------------------
 *
 * Minimum holds an answer only when the objective is unimodal along the
 * segment. When it is not, the search still returns the local minimum of the
 * last bracket, and reports nothing. The pivots themselves are never
 * evaluated: the two evaluations which precede the first iteration are
 * interior points, so a minimum at a pivot is approached only as the bracket
 * shrinks onto it.
 *
 * @param[in] Dimension
 *     Number of components of the pivots and of Minimum. Dimension = 0 is a
 *     valid no-op: no callback is invoked and nothing is written, not even
 *     the NaN fill.
 *
 * @param[in] LeftPivot
 *     Pointer to the Dimension doubles of the first end of the segment. Must
 *     not be NULL, and its memory is read at every iteration.
 *
 * @param[in] RightPivot
 *     Pointer to the Dimension doubles of the second end of the segment.
 *     Must not be NULL, and its memory is read at every iteration.
 *
 * @param[in] ObjectiveFunction
 *     Objective whose minimum is sought. Must not be NULL.
 *
 * @param[in] MaxIterations
 *     Upper bound on the number of golden-section iterations. 0 is a
 *     rejection: the routine returns without evaluating the objective.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the width of the bracket, in the units of the
 *     points: the search stops when the largest difference between the ends of
 *     the bracket over the components is less than or equal to this value. A
 *     value which is not smaller than the length of the segment stops before
 *     the first evaluation, and Minimum is the midpoint of the segment. The
 *     value is NOT a bound on |f(Minimum)|, so the caller validates Minimum. A
 *     negative or NaN value is a rejection.
 *
 * @param[out] Minimum
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
 *     None. The outcome is reported through Minimum.
 *
 * @note
 * - The returned point is the midpoint of the last bracket, an estimate: the
 *   objective need not have been evaluated there, and no exact answer can be
 *   reached, because rho is irrational.
 * - The memory of the pivots is read at every iteration: an objective which
 *   writes to it through UserContext alters the search.
 * - Minimum must not overlap either pivot: the NaN fill runs before the
 *   pivots are read, so an overlapping buffer would be overwritten and the
 *   call would report the absence of an answer. For a Fortran caller the
 *   question does not arise, because the result is a new array.
 * - A pivot which is not finite (NaN or infinite), a negative or NaN
 *   Tolerance, a MaxIterations of 0 and a NULL ObjectiveFunction are
 *   rejections: the objective is not evaluated, and Minimum is NaN.
 * - An objective which returns NaN terminates the search: Minimum is NaN.
 * - A minimum at a pivot is approached from the interior: the pivot is never
 *   evaluated, and Minimum approaches it as the bracket shrinks.
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
 * Brent's root finding method along the segment. The bracket is kept in the
 * parameter t, between two estimates which lie on opposite sides of zero, and
 * the two estimates which precede the current one are kept as well. Every
 * iteration forms the next estimate by inverse quadratic interpolation through
 * the three most recent estimates, by the secant through the two most recent
 * ones when the first and the third coincide, and by bisection of the bracket
 * whenever the interpolated point falls outside the bracket or is not shorter
 * than the step which produced the current estimate. Interpolation is
 * therefore always backed by bisection: the bracket shrinks at least as fast
 * as bisection makes it shrink, and much faster when the objective is smooth.
 *
 * A point of the segment is the convex combination (1 - t) * LeftPivot +
 * t * RightPivot, which lies between the pivots and never forms their
 * difference, so the points cannot overflow. The values are compared through
 * their sign, never through their product.
 *
 * Two points are evaluated before the first iteration and at most one per
 * iteration afterwards.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * In exact arithmetic the bracket always holds a sign change, and the search
 * falls back on bisection whenever the interpolation cannot be trusted, so the
 * estimate approaches a sign change at least as fast as a halving of the
 * segment and, on a smooth objective, at the superlinear rate of the
 * interpolation.
 *
 * The search stops at whichever of the following conditions is met first: a
 * pivot at which the objective vanishes, a value which is NaN, a residual
 * |f(Root)| less than or equal to Tolerance, a bracket which has reached the
 * spacing of the parameter, or MaxIterations iterations. Root is the last
 * estimate in every case, and rounding is the only reason a tighter point may
 * not be reached.
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
 *     Upper bound on the number of iterations. 0 is a rejection: the routine
 *     returns without evaluating the objective.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the residual |f(Root)|, checked at every estimate.
 *     The value only decides when to stop; it is not a bound on the distance
 *     between Root and a zero of f. A negative or NaN value is a rejection.
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
 * - Root is an estimate, not an exact answer: it is the last point which the
 *   search evaluated inside a bracket that still held the sign change.
 * - The pivots are copied before the first evaluation, so an objective which
 *   reuses the buffers of the caller through UserContext cannot alter the
 *   search.
 * - Root must not overlap either pivot: the NaN fill runs before the pivots
 *   are read, so an overlapping buffer would be overwritten and the call
 *   would report the absence of an answer. For a Fortran caller the question
 *   does not arise, because the result is a new array.
 * - An objective which returns NaN terminates the search: Root is NaN.
 * - A pivot which is not finite (NaN or infinite), a pivot pair on the same
 *   side of zero, a negative or NaN Tolerance and a MaxIterations of 0 are
 *   rejections: the objective is not evaluated, and Root is NaN.
 * - The values at the pivots are compared through their sign, never through
 *   their product, so brackets whose product would over- or underflow are
 *   accepted.
 * - A zero at a pivot is an exact answer and is returned unchanged.
 * - The points evaluated by the objective lie on the segment: the
 *   interpolation never extrapolates.
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
 * Brent's method for minimization along the segment. The bracket is kept in
 * the parameter t together with three estimates: x, the best point found, and
 * w and v, the two points which precede it. Every iteration fits the parabola
 * through the three, and evaluates its vertex when the vertex lies inside the
 * bracket and is nearer to x than the step which produced w. Otherwise it
 * takes the golden-section step
 *
 *     d = c * e,     c = (3 - sqrt(5)) / 2, near 0.382,
 *
 * towards the larger part of the bracket. The point whose value is larger is
 * discarded together with the part of the bracket beyond it, so the bracket
 * shrinks and the three estimates keep their meaning: the parabola is replaced
 * by another as soon as the objective departs from it.
 *
 * A point of the segment is the convex combination (1 - t) * LeftPivot +
 * t * RightPivot, which lies between the pivots and never forms their
 * difference, so the points cannot overflow. The length of the segment is
 * measured in halves for the same reason, and is capped at the largest value
 * of the type when the segment is wider than its range. One point is evaluated
 * before the first iteration and at most one per iteration afterwards.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * Whenever the parabola is rejected the bracket keeps a fraction rho of its
 * width, with rho = (sqrt(5) - 1) / 2, exactly as in golden-section search, so
 * the worst case of that search is the worst case of this one: after k
 * iterations the bracket is at most rho^k times the segment. A smooth
 * objective reaches the tolerance far sooner, because the parabolic step is
 * superlinear and the estimate then has the accuracy of the model and not of
 * the bracket.
 *
 * The search stops at whichever of these conditions is met first: a bracket
 * whose width is less than or equal to twice Tolerance, a value which is NaN, a
 * step which can neither move the estimate nor narrow the bracket, or
 * MaxIterations iterations. In the first case the returned point lies within
 * twice Tolerance of a minimizer, and rounding is the only reason why that
 * bound may not be reached.
 *
 * Tolerance bounds the width of the bracket in the units of the points, and
 * not the value of the objective: it is a stopping rule, not a promise about
 * the minimum, so the caller validates Minimum. No step of the search lands
 * nearer to the estimate than Tolerance, so the bracket is measured against
 * twice the value: the search stops one bracket wider than the tolerance
 * itself instead of walking the last of it one tolerance at a time. A
 * Tolerance which is not smaller than half the length of the segment stops
 * before anything is evaluated, and Minimum is the midpoint of the segment.
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
 *     Number of components of the pivots and of Minimum. Dimension = 0 is a
 *     valid no-op: no callback is invoked and nothing is written, not even
 *     the NaN fill.
 *
 * @param[in] LeftPivot
 *     Pointer to the Dimension doubles of the first end of the segment. Must
 *     not be NULL, and its memory is read at every iteration.
 *
 * @param[in] RightPivot
 *     Pointer to the Dimension doubles of the second end of the segment.
 *     Must not be NULL, and its memory is read at every iteration.
 *
 * @param[in] ObjectiveFunction
 *     Objective whose minimum is sought. Must not be NULL.
 *
 * @param[in] MaxIterations
 *     Upper bound on the number of iterations. 0 is a rejection: the routine
 *     returns without evaluating the objective.
 *
 * @param[in] Tolerance
 *     Non-negative bound on the width of the bracket, in the units of the
 *     points: the search stops once the width is not greater than twice this
 *     value, because no step lands nearer to the estimate than the tolerance
 *     itself. A value which is not smaller than half the length of the segment
 *     stops before the first evaluation, and Minimum is the midpoint of the
 *     segment. The value is NOT a bound on |f(Minimum)|, so the caller
 *     validates Minimum. A negative or NaN value is a rejection.
 *
 * @param[out] Minimum
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
 *     None. The outcome is reported through Minimum.
 *
 * @note
 * - The returned point was evaluated by the objective, unless the search
 *   stopped before the first evaluation, in which case it is the midpoint of
 *   the segment.
 * - Minimum must not overlap either pivot: the NaN fill runs before the
 *   pivots are read, so an overlapping buffer would be overwritten and the
 *   call would report the absence of an answer. For a Fortran caller the
 *   question does not arise, because the result is a new array.
 * - The memory of the pivots is read at every iteration: an objective which
 *   writes to it through UserContext alters the search.
 * - A pivot which is not finite (NaN or infinite), a negative or NaN
 *   Tolerance, a MaxIterations of 0 and a NULL ObjectiveFunction are
 *   rejections: the objective is not evaluated, and Minimum is NaN.
 * - An objective which returns NaN terminates the search: Minimum is NaN.
 * - A zero-length space is a valid no-op: the objective is not evaluated and
 *   Minimum is empty, so the caller reads the size to tell the case apart
 *   from a rejection.
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
