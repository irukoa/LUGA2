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
