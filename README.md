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

Root finding and unimodal minimization along the segment

$$
\textbf{x}(t) = \textbf{a} + t\left(\textbf{b} - \textbf{a}\right), \qquad t \in [0, 1],
$$

which joins two pivots $\textbf{a}$ and $\textbf{b}$ of $\mathbb{R}^n$:

- **`BisectionRootFinding`** — sign change of a function $f(\textbf{x})$ which takes opposite signs at the pivots. One halving of the segment per iteration, so after $k$ iterations the returned point lies within $|\textbf{b} - \textbf{a}| / 2^k$ of a sign change, in any norm.
- **`BrentRootFinding`** — the same sign change by inverse quadratic interpolation and secant steps, backed by bisection: superlinear on a smooth function, and at most twice the iterations of bisection in the worst case.
- **`GoldenSectionMinimization`** — minimum of a unimodal function. The bracket keeps a fraction $\rho = (\sqrt{5} - 1)/2$ of its width per iteration, so after $k$ iterations the returned point lies within $\rho^k |\textbf{b} - \textbf{a}| / 2$ of a minimizer, in the largest component.
- **`BrentMinimization`** — the same minimum by successive parabolic interpolation, with the golden-section step whenever the parabola is not trusted: superlinear on a smooth function, and no worse than golden-section search up to a small factor in the iteration count.

Only the sign change, or the unimodality, is assumed: neither continuity nor differentiability is required. The bracket is kept in the parameter $t$, so no point or width can overflow, and a point of the segment is a convex combination of the pivots. The objective is a callback, optionally carrying a caller-defined context. No error code is returned: a rejected argument, a segment which does not bracket a sign change, a function which is not unimodal, or a NaN value leaves the result filled with NaN, and `Tolerance` is a stopping rule rather than a promise, so the caller validates the result. FOA keeps no global state and needs no storage beyond a few work arrays sized by the point, so concurrent calls are safe provided the callback is thread-safe.

## Message Coding Interface (MCI)

Bit-level packing and unpacking of fixed-width unsigned fields into a byte buffer. A textual format is compiled once into an immutable `MCICompiledFormat`, which records each field's width and offset and the total message size. The same compiled format can then encode or decode fields in either bit order, selected per call by `MSB_FIRST` or `LSB_FIRST`.

The format grammar is

```text
Format  ::= Element ('|' Element)*
Element ::= Digits ['R' '(' Format ')']
```

`Digits` is a positive decimal field width in bits. A width immediately followed by `R(` begins a repetition group, which is closed by the matching `)`. Repetition groups are expanded at compile time, so every field has a distinct zero-based `FieldId`; for example, `8|4|2R(3)` describes fields of widths 8, 4, 3 and 3, numbered 0 through 3. Repetition counts and nesting are limited to 64, and the total number of expanded fields is limited to 1024.

MCI's Fortran buffer arguments use `unsigned(C_UINT8_T)`. With GFortran, application code that uses MCI must be compiled with `-funsigned` (the library itself uses this flag).

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
  - `MessageCodingInterface.h`, `MessageCodingInterface.F90` (MCI)

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
