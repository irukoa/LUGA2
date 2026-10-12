#!/usr/bin/env bash

# LUGA2 installation script.

set -euo pipefail

# Defaults.
PREFIX="/usr/local"
DESTDIR=""
CC=""
FC=""
CFLAGS=""
FFLAGS=""

Usage() {
	cat <<EOF
Usage: $0 [VAR=VALUE ...]

Initializes external tools, builds the library in release mode, and
installs the libraries into \${DESTDIR}\${LIBDIR} and the public headers
and Fortran interface modules into \${DESTDIR}\${INCDIR}.

Requires GNU Make, GNU Binutils (ar, nm, objcopy), a GCC-compatible C and
Fortran toolchain (e.g. GCC/GFortran), a POSIX shell, and standard
utilities (mkdir, cp, mv, rm, printf, sed, awk). Dependency generation
invokes gcc directly.

Recognized variables:
  PREFIX   Installation prefix.                (default: /usr/local)
  DESTDIR  Staging directory, for packagers.   (default: empty)
  LIBDIR   Library destination.                (default: \$PREFIX/lib)
  INCDIR   Header destination.                 (default: \$PREFIX/include)
  CC       C compiler (forwarded to make).     (default: make's default)
  FC       Fortran compiler (forwarded).       (default: make's default)
  CFLAGS   C compiler flags (forwarded).       (default: make's default)
  FFLAGS   Fortran compiler flags (forwarded). (default: make's default)

Values containing spaces must be quoted, e.g.:
  ./Install.sh 'CFLAGS=-O2 -g -Werror'
EOF
}

for Arg in "$@"; do
	case "$Arg" in
	PREFIX=*) PREFIX="${Arg#PREFIX=}" ;;
	DESTDIR=*) DESTDIR="${Arg#DESTDIR=}" ;;
	LIBDIR=*) LIBDIR="${Arg#LIBDIR=}" ;;
	INCDIR=*) INCDIR="${Arg#INCDIR=}" ;;
	CC=*) CC="${Arg#CC=}" ;;
	FC=*) FC="${Arg#FC=}" ;;
	CFLAGS=*) CFLAGS="${Arg#CFLAGS=}" ;;
	FFLAGS=*) FFLAGS="${Arg#FFLAGS=}" ;;
	-h | --help)
		Usage
		exit 0
		;;
	*)
		echo "Error: unrecognized argument: '$Arg'." >&2
		echo "Run '$0 --help' for usage." >&2
		exit 1
		;;
	esac
done

# Run from the repository root, wherever the script is invoked from.
cd "$(cd "$(dirname "$0")" && pwd -P)"

# Sanity checks.
command -v make >/dev/null 2>&1 || {
	echo "Error: 'make' not found." >&2
	exit 1
}
if [ -n "$CC" ] && ! command -v "${CC%% *}" >/dev/null 2>&1; then
	echo "Error: compiler '$CC' not found." >&2
	exit 1
fi
if [ -n "$FC" ] && ! command -v "${FC%% *}" >/dev/null 2>&1; then
	echo "Error: compiler '$FC' not found." >&2
	exit 1
fi

# Defaults derived from PREFIX.
LIBDIR="${LIBDIR:-$PREFIX/lib}"
INCDIR="${INCDIR:-$PREFIX/include}"

# Forward the provided toolchain variables to make.
MAKEVARS=()
[ -n "$CC" ] && MAKEVARS+=("CC=$CC")
[ -n "$FC" ] && MAKEVARS+=("FC=$FC")
[ -n "$CFLAGS" ] && MAKEVARS+=("CFLAGS=$CFLAGS")
[ -n "$FFLAGS" ] && MAKEVARS+=("FFLAGS=$FFLAGS")

echo "LUGA2 installer"
echo "  PREFIX  : $PREFIX"
echo "  DESTDIR : $DESTDIR"
echo "  LIBDIR  : ${DESTDIR}${LIBDIR}"
echo "  INCDIR  : ${DESTDIR}${INCDIR}"
echo "  CC      : ${CC:-make default}"
echo "  FC      : ${FC:-make default}"
echo "  CFLAGS  : ${CFLAGS:-make default}"
echo "  FFLAGS  : ${FFLAGS:-make default}"
echo

# Clean the build tree and build the release profile.
make clean
make PROFILE=release ${MAKEVARS[@]+"${MAKEVARS[@]}"}

# Install the libraries.
mkdir -p "${DESTDIR}${LIBDIR}"
cp bin/libLUGA2.a "${DESTDIR}${LIBDIR}/libLUGA2.a"
cp bin/libLUGA2.so "${DESTDIR}${LIBDIR}/libLUGA2.so"
chmod 644 "${DESTDIR}${LIBDIR}/libLUGA2.a" "${DESTDIR}${LIBDIR}/libLUGA2.so"

# Install the public headers and Fortran interface modules.
mkdir -p "${DESTDIR}${INCDIR}"
cp src/MAC/MultidimensionalArrayContainers.h "${DESTDIR}${INCDIR}/MultidimensionalArrayContainers.h"
cp src/MAC/MultidimensionalArrayContainers.M90 "${DESTDIR}${INCDIR}/MultidimensionalArrayContainers.F90"
cp src/EINT/ExtrapolationIntegration.h "${DESTDIR}${INCDIR}/ExtrapolationIntegration.h"
cp src/EINT/ExtrapolationIntegration.M90 "${DESTDIR}${INCDIR}/ExtrapolationIntegration.F90"
chmod 644 "${DESTDIR}${INCDIR}/MultidimensionalArrayContainers.h" \
  "${DESTDIR}${INCDIR}/MultidimensionalArrayContainers.F90" \
  "${DESTDIR}${INCDIR}/ExtrapolationIntegration.h" \
  "${DESTDIR}${INCDIR}/ExtrapolationIntegration.F90"

echo
echo "Installed:"
echo "  ${DESTDIR}${LIBDIR}/libLUGA2.a"
echo "  ${DESTDIR}${LIBDIR}/libLUGA2.so"
echo "  ${DESTDIR}${INCDIR}/MultidimensionalArrayContainers.h"
echo "  ${DESTDIR}${INCDIR}/MultidimensionalArrayContainers.F90"
echo "  ${DESTDIR}${INCDIR}/ExtrapolationIntegration.h"
echo "  ${DESTDIR}${INCDIR}/ExtrapolationIntegration.F90"
echo
