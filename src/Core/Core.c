#include "src/Core/Core.h"

APIDEF(void *,
       Core_Malloc,
       const size_t Size) {
  if (Size == 0) {
    return NULL;
  }
  return (void *)malloc(Size);
}

APIDEF(void *,
       Core_Realloc,
       void        *Ptr,
       const size_t NewSize) {
  if (NewSize == 0) {
    return NULL;
  }
  return (void *)realloc(Ptr, NewSize);
}

APIDEF(bool,
       Core_AddOverflow,
       const size_t A,
       const size_t B,
       size_t      *Res) {
  return __builtin_add_overflow(A, B, Res);
}

APIDEF(bool,
       Core_MulOverflow,
       const size_t A,
       const size_t B,
       size_t      *Res) {
  return __builtin_mul_overflow(A, B, Res);
}

// Fortran-side wrappers.

void FortranFree(void *Ptr) { free(Ptr); }

void *FortranMalloc(const size_t Size) { return Core_Malloc(Size); }

void *FortranRealloc(void        *Ptr,
                     const size_t NewSize) {
  return Core_Realloc(Ptr, NewSize);
}

bool FortranAddOverflow(const size_t A,
                        const size_t B,
                        size_t      *Res) {
  return Core_AddOverflow(A, B, Res);
}

bool FortranMulOverflow(const size_t A,
                        const size_t B,
                        size_t      *Res) {
  return Core_MulOverflow(A, B, Res);
}
