#ifndef ExtrapolationIntegration_CH
#define ExtrapolationIntegration_CH

#include <stddef.h>

/**
 * @brief Computes the normalized integral (mean value) of a function on a
 * uniform grid on the interval [a, b] using extrapolated quadrature.
 *
 * @details
 * EINT computes the normalized integral (mean value) of a function over a
 * uniform grid on the interval [a, b]:
 *
 *     I = (1 / (b - a)) * ∫_a^b f(x) dx.
 *
 * -------------------------------------------------------------------------
 * NUMERICAL METHOD
 * -------------------------------------------------------------------------
 *
 * The input array represents samples of a function evaluated on a uniform
 * grid:
 *
 *     x_i = a + (i - 1) * h,   i = 1, ..., N;
 *     h   = (b - a) / (N - 1),
 *
 * with:
 *
 *     Array[i - 1] = f(x_i).
 *
 * The method is based on a Richardson extrapolation scheme (Romberg-type
 * integration generalized to arbitrary refinement bases).
 *
 * -------------------------------------------------------------------------
 * REFINEMENT BASE (Base)
 * -------------------------------------------------------------------------
 *
 * The argument Base controls the extrapolation structure:
 *
 * - Base = p, where p is a prime number.
 * - If Base is not provided or invalid, Base = 2 is used.
 *
 * The convergence behavior depends on the factorization of (N - 1):
 *
 *     N - 1 = p^(n_p) * k,   with p not dividing k,
 *
 * the extrapolation can be performed to level n_p using base p.
 * The achievable convergence order increases with n_p.
 *
 * -------------------------------------------------------------------------
 * ACCURACY
 * -------------------------------------------------------------------------
 *
 * If the function f is sufficiently smooth, the expected error scales as:
 *
 *     O(h^(2 * n_p)),
 *
 * where n_p is the highest exponent of Base dividing (N - 1).
 *
 * -------------------------------------------------------------------------
 * NOTES
 * -------------------------------------------------------------------------
 *
 * - The algorithm requires a uniform grid including both endpoints.
 * - Larger powers of Base dividing (N - 1) improve convergence.
 * - This method generalizes classical Romberg integration (Base = 2).
 *
 * @param[in] Count
 *     Number of samples in Array.
 *
 * @param[in] Array
 *     Function values f(x_i) sampled on a uniform grid.
 *     Must contain Count elements.
 *
 * @param[in] Base
 *     Extrapolation base. Must be a prime number.
 *     If invalid, a default value of 2 is used.
 *
 * @return
 * Approximated value of the normalized integral.
 */
double EINT(const size_t Count,
            const double Array[Count],
            const size_t Base);

/**
 * - Parallel version (OMP).
 */
double EINT_P(const size_t Count,
              const double Array[Count],
              const size_t Base);

#endif
