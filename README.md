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

OpenMP is opt-in and provided through the Fortran compiler flags:

```bash
./Install.sh 'FFLAGS=-O3 -std=gnu -fPIC -fopenmp'
```

Enabling OpenMP adds a runtime dependency on the OpenMP runtime (e.g. `libgomp`).

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

## Build targets

- `make` (default) — builds `libLUGA2.a` and `libLUGA2.so` with the debug profile.
- `make PROFILE=release` — builds the library with the release profile.
- `make clean` — removes the build products (`obj/`, `bin/`, `discard/`) and test artifacts.
- `make test`, `make runtest`, `make coverage` — build and run the test suite.

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
