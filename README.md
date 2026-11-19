[![DOI](https://zenodo.org/badge/1415859549.svg)](https://doi.org/10.5281/zenodo.23647496)

# LUGA2 Modules

LUGA2 provides the following modules.

## Multidimensional Array Containers (MAC)

Multidimensional array indexing functionality.

Converts between flat linear indexes and multidimensional indexes:
- C: useful to avoid multidimensional array allocations.
- Fortran: useful to promote array rank to a runtime property.

Supports both column-major (`COLUMN_MAJOR`) and row-major (`ROW_MAJOR`) memory layouts.

## Extrapolation Integration (EINT)

Computes the normalized integral of a function over a uniformly sampled interval:

$$
I = \frac{1}{b-a}\int_{a}^{b}f(x)dx.
$$

The input is a uniformly spaced `N`-element array of function values including both interval endpoints. Integration is performed using a Richardson-extrapolation scheme that supports arbitrary prime refinement bases.

The `Base` parameter controls the extrapolation structure. Accuracy improves when `N - 1` contains larger powers of `Base`. For smooth functions, the method achieves progressively higher-order convergence.

### Parallel Execution

OpenMP is opt-in for `EINT_P`, and provided through the Fortran compiler flags:

```bash
./Install.sh 'FFLAGS=-O3 -std=gnu -fPIC -fopenmp'
```

Enabling OpenMP adds a runtime dependency on the OpenMP runtime (e.g. `libgomp`).

## LAPACK Deferred Wrappers (LDW)

Deferred LAPACK wrappers: LAPACK procedures are supplied by the application at runtime rather than linked into LUGA2.

Provides Hermitian diagonalization (`HermitianEigendecomposition`), Schur decomposition (`SchurDecomposition`), singular value decomposition (`SVDecomposition`), and generalized Schur decomposition (`GeneralizedSchurDecomposition`) through a caller-supplied `ZHEEVD`/`ZGEES`/`ZGESDD`/`ZGGES` procedure, and consecutive eigenvalue grouping (`DegeneracyGrouping`). Applications using LDW must provide a compatible LAPACK/BLAS implementation and link it themselves.

## Graph Topologies and Shortest-path Algorithms (GRTP)

Directed graph topology construction and shortest-path algorithms.

- **Topology** — `GraphSupportBuilder` incrementally builds a directed graph by adding and removing edges (self-loops allowed, parallel edges rejected) and finalizes it into an immutable `GraphSupport` instance with compact, sorted neighbor lists enabling efficient traversal and binary-search edge lookup.
- **Algorithms** — Dijkstra (single- and multi-source), bidirectional Dijkstra (with shortest-path reconstruction), and Bellman–Ford (with optional reachable negative-cycle detection). A reusable `GraphWorkspace` holds all temporary and output data to avoid repeated allocations across computations.

## Tight-Binding Crystal Interpolation (TBCI)

Fourier interpolation of a lattice-resolved tight-binding model. A `TBCrystal` stores the set of direct lattice vectors $`\{\textbf{a}_i\}`$, the set of integer site offsets $\{\textbf{n}\}$, the lattice-resolved Hamiltonian blocks $H_{nm}(\textbf{R})$ and the lattice-resolved Berry connection blocks $A^j_{nm}(\textbf{R})$, where $n$ and $m$ index the bands, $j$ is a Cartesian component and $\textbf{R}$ is a lattice translation

$$
\textbf{R} = \sum_{i}\textbf{a}_i n_i.
$$

The Bloch Hamiltonian and the Berry connection at a reciprocal-space point $\textbf{k}$ are the Fourier sums

$$
H(\textbf{k}) = \sum_{R}H(\textbf{R})e^{i\textbf{k}\textbf{R}}, \qquad A^j(\textbf{k}) = \sum_{R}A^j(\textbf{R})e^{i\textbf{k}\textbf{R}}.
$$

`TBCrystalHamiltonian` and `TBCrystalPositionOperator` evaluate $H(\textbf{k})$ and $A^j(\textbf{k})$, or any Cartesian k-derivative of them of order `Derivative`, and return the result as a flat array plus its runtime shape in column-major order.

## Functional Optimization Algorithms (FOA)

`BisectionRootFinding` locates a sign change of an objective function $f(\textbf{x})$ ($\mathbb{R}^n\to\mathbb{R}$) on the segment

$$
\textbf{x}(t) = \textbf{a} + t\left(\textbf{b} - \textbf{a}\right), \qquad t \in [0, 1],
$$

and returns the point at which the sign changes. Every iteration evaluates the objective at the midpoint of the current bracket and keeps the half which still holds the sign change, so after $k$ halvings the returned point lies within $|\textbf{b} - \textbf{a}| / 2^k$ of a sign change, in any norm.

- **Assumptions** — only that the objective takes opposite signs at the two pivots; neither continuity nor differentiability is required.
- **Numerical robustness** — the pivot values are compared through their sign, never through their product. A bracket which cannot be split is detected before evaluating. A NaN value terminates the search.
- **Outcome** — no error code is returned. A rejected argument, a segment which does not bracket a sign change, or a NaN value leaves the result filled with NaN. `Tolerance` is a stopping rule on $|f|$ for this search, not a promise, so the caller validates the returned point.

`GoldenSectionMinimization` locates the minimum of an objective function on the same segment, and returns the point at which the objective is smallest.

- **Assumptions** — only that the objective is unimodal along the segment. Neither continuity nor differentiability is required.
- **Numerical robustness** — the bracket is kept in the parameter $t$, so its width is a fraction of the segment and cannot overflow, and a point of the segment is a convex combination of the pivots. A NaN value terminates the search.
- **Outcome** — no error code is returned. A rejected argument or a NaN value leaves the result filled with NaN, and an objective which is not unimodal reports the minimum of the last bracket. `Tolerance` bounds the width of the bracket, in the units of the points, and not the value of the objective: the returned point lies within `Tolerance` / 2 of a minimizer of the objective on the segment, so the caller validates it.

`BrentRootFinding` locates a sign change of an objective function $f(\textbf{x})$ on the same segment, and returns the point at which the sign changes. Every iteration interpolates the three most recent estimates: by inverse quadratic interpolation through the three, or by the secant through the two most recent ones when the first and the third coincide. The interpolated point is replaced by the midpoint of the bracket whenever it leaves the bracket or is not shorter than the step which produced the current estimate, so the convergence is superlinear on a smooth objective while the worst case remains that of `BisectionRootFinding`.

- **Assumptions** — only that the objective takes opposite signs at the two pivots; neither continuity nor differentiability is required.
- **Numerical robustness** — the bracket is kept in the parameter $t$, a point of the segment is a convex combination of the pivots, so it cannot overflow, and the pivot values are compared through their sign, never through their product. A NaN value terminates the search.
- **Outcome** — no error code is returned. A rejected argument, a segment which does not bracket a sign change, or a NaN value leaves the result filled with NaN. `Tolerance` is a stopping rule on $|f|$ for this search, not a promise, so the caller validates the returned point.

`BrentMinimization` locates the minimum of an objective function on the same segment, and returns the point at which the objective is smallest. Every iteration fits the parabola through the three most recent estimates and evaluates its vertex when that vertex lies inside the bracket and is nearer to the best estimate than the step which produced the second one; otherwise it takes the golden-section step of `GoldenSectionMinimization`, towards the larger part of the bracket. A smooth objective therefore converges superlinearly, while the worst case remains that of golden-section search.

- **Assumptions** — only that the objective is unimodal along the segment. Neither continuity nor differentiability is required.
- **Numerical robustness** — the bracket is kept in the parameter $t$, so its width is a fraction of the segment and cannot overflow, and a point of the segment is a convex combination of the pivots. An objective whose parabola degenerates, or whose step cannot move the estimate, terminates the search instead of iterating.
- **Outcome** — no error code is returned. A rejected argument or a NaN value leaves the result filled with NaN, and an objective which is not unimodal reports the best point of the last bracket. `Tolerance` bounds the width of the bracket, in the units of the points, and not the value of the objective: no step lands nearer to the estimate than `Tolerance`, so the search stops once the bracket is no wider than twice that value, and the returned point lies within `2 * Tolerance` of a minimizer of the objective on the segment, so the caller validates it.

The objective is a callback, optionally carrying a caller-defined context. FOA keeps no global state and needs no storage beyond a few work arrays sized by the point, so concurrent calls are safe provided the callback is thread-safe.

# Building and Installation

LUGA2 provides:
- C header files for C applications.
- Fortran interface modules for Fortran applications.
- Static and/or shared library binaries containing the implementation.

## Requirements

Building and installing require:
- GNU Make.
- GNU Binutils (`ar`, `nm`, `objcopy`).
- A GCC-compatible C and Fortran toolchain (e.g. GCC/GFortran); dependency generation invokes `gcc` directly.
- A POSIX shell and standard utilities (`mkdir`, `cp`, `mv`, `rm`, `printf`, `sed`, `awk`).
- A Fortran2023 (ISO/IEC 1539:2023) compliant compiler (GFortran preferred).

## Installing

Release installation is orchestrated by the `Install.sh` helper script.

```bash
./Install.sh                         # install to /usr/local
./Install.sh -h                      # full usage, variables and examples
./Install.sh PREFIX="$HOME/.local"
./Install.sh DESTDIR="$PWD/stage"
```

After installation, the generated artifacts are installed as follows:
- Libraries `libLUGA2.a` and `libLUGA2.so` in `$(DESTDIR)$(LIBDIR)`.
- Public headers and Fortran interface modules in `$(DESTDIR)$(INCDIR)`:
  - `MultidimensionalArrayContainers.h`, `MultidimensionalArrayContainers.F90` (MAC)
  - `ExtrapolationIntegration.h`, `ExtrapolationIntegration.F90` (EINT)
  - `LapackDeferredWrappers.h`, `LapackDeferredWrappers.F90` (LDW)
  - `GraphUtilities.h`, `GraphUtilities.F90` (GRTP)
  - `TightBindingCrystalInterpolation.h`, `TightBindingCrystalInterpolation.F90` (TBCI)
  - `FunctionalOptimizationAlgorithms.h`, `FunctionalOptimizationAlgorithms.F90` (FOA)

## Build targets

- `make` (default) — builds `libLUGA2.a` and `libLUGA2.so` with the debug profile.
- `make PROFILE=release` — builds the library with the release profile.
- `make clean` — removes the build products (`obj/`, `bin/`, `discard/`) and test artifacts.
- `make test`, `make runtest`, `make coverage` — build and run the test suite. Note: these require BLAS and LAPACK.

## Using the Library from C

Include the appropriate public header files in your source code and link against the generated library.

Refer to the public header files for the complete API definition, available functions, data structures, constants, and usage notes.

## Using the Library from Fortran

Compile the provided Fortran interface modules as part of your project and link against the generated library.

The public interface modules define the available derived types, procedures, and interfaces exposed by the library. They are intended to be used directly by applications and should be considered part of the library documentation.

Users are encouraged to consult the interface modules when exploring the API, as they provide complete procedure signatures, argument intents, result definitions, and type declarations.

## Notes

When using the Fortran interface, it is recommended that the library and application are built using the same compiler toolchain and compatible compilation settings.

The public interface files are designed to remain stable and separate from the implementation. Applications should depend only on the distributed headers and Fortran interface modules and should not rely on internal implementation details.
