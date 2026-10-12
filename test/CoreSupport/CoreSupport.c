#include "test/CoreSupport/CoreSupport.h"
#include <stdlib.h>

typedef struct FallibleProcedureConfiguration {
  size_t CallCount;
  size_t FailAt;
  bool   SetToFail;
} FallibleProcedureConfiguration;

// Malloc.
static FallibleProcedureConfiguration FallibleMallocCfg = {.CallCount = 0,
                                                           .FailAt    = 0,
                                                           .SetToFail = false};

void ResetFallibleMalloc(void) {
  FallibleMallocCfg.CallCount = 0;
  FallibleMallocCfg.FailAt    = 0;
  FallibleMallocCfg.SetToFail = false;
}
void SetFallibleMalloc(const size_t FailAt) {
  FallibleMallocCfg.CallCount = 0;
  FallibleMallocCfg.FailAt    = FailAt;
  FallibleMallocCfg.SetToFail = true;
}
void *FallibleMalloc(const size_t Size) {
  FallibleMallocCfg.CallCount++;
  if ((FallibleMallocCfg.SetToFail) &&
      (FallibleMallocCfg.CallCount == FallibleMallocCfg.FailAt)) {
    ResetFallibleMalloc();
    return NULL;
  } else {
    if (Size == 0) {
      return NULL;
    }
    return (void *)malloc(Size);
  }
}

// Realloc.
static FallibleProcedureConfiguration FallibleReallocCfg = {.CallCount = 0,
                                                            .FailAt    = 0,
                                                            .SetToFail = false};

void ResetFallibleRealloc(void) {
  FallibleReallocCfg.CallCount = 0;
  FallibleReallocCfg.FailAt    = 0;
  FallibleReallocCfg.SetToFail = false;
}
void SetFallibleRealloc(const size_t FailAt) {
  FallibleReallocCfg.CallCount = 0;
  FallibleReallocCfg.FailAt    = FailAt;
  FallibleReallocCfg.SetToFail = true;
}
void *FallibleRealloc(void        *Ptr,
                      const size_t NewSize) {
  FallibleReallocCfg.CallCount++;
  if ((FallibleReallocCfg.SetToFail) &&
      (FallibleReallocCfg.CallCount == FallibleReallocCfg.FailAt)) {
    ResetFallibleRealloc();
    return NULL;
  } else {
    if (NewSize == 0) {
      return NULL;
    }
    return (void *)realloc(Ptr, NewSize);
  }
}

// AddOverflow.
static FallibleProcedureConfiguration FallibleAddOverflowCfg = {.CallCount = 0,
                                                                .FailAt    = 0,
                                                                .SetToFail =
                                                                    false};

void ResetFallibleAddOverflow(void) {
  FallibleAddOverflowCfg.CallCount = 0;
  FallibleAddOverflowCfg.FailAt    = 0;
  FallibleAddOverflowCfg.SetToFail = false;
}
void SetFallibleAddOverflow(const size_t FailAt) {
  FallibleAddOverflowCfg.CallCount = 0;
  FallibleAddOverflowCfg.FailAt    = FailAt;
  FallibleAddOverflowCfg.SetToFail = true;
}
bool FallibleAddOverflow(const size_t A,
                         const size_t B,
                         size_t      *Res) {
  FallibleAddOverflowCfg.CallCount++;
  if ((FallibleAddOverflowCfg.SetToFail) &&
      (FallibleAddOverflowCfg.CallCount == FallibleAddOverflowCfg.FailAt)) {
    ResetFallibleAddOverflow();
    return true;
  } else {
    return __builtin_add_overflow(A, B, Res);
  }
}

// MulOverflow.
static FallibleProcedureConfiguration FallibleMulOverflowCfg = {.CallCount = 0,
                                                                .FailAt    = 0,
                                                                .SetToFail =
                                                                    false};

void ResetFallibleMulOverflow(void) {
  FallibleMulOverflowCfg.CallCount = 0;
  FallibleMulOverflowCfg.FailAt    = 0;
  FallibleMulOverflowCfg.SetToFail = false;
}
void SetFallibleMulOverflow(const size_t FailAt) {
  FallibleMulOverflowCfg.CallCount = 0;
  FallibleMulOverflowCfg.FailAt    = FailAt;
  FallibleMulOverflowCfg.SetToFail = true;
}
bool FallibleMulOverflow(const size_t A,
                         const size_t B,
                         size_t      *Res) {
  FallibleMulOverflowCfg.CallCount++;
  if ((FallibleMulOverflowCfg.SetToFail) &&
      (FallibleMulOverflowCfg.CallCount == FallibleMulOverflowCfg.FailAt)) {
    ResetFallibleMulOverflow();
    return true;
  } else {
    return __builtin_mul_overflow(A, B, Res);
  }
}
