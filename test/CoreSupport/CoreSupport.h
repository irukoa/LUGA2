#ifndef LUGA2_Core_Support_CH
#define LUGA2_Core_Support_CH

#include <stddef.h>

// Malloc.
void  ResetFallibleMalloc(void);
void  SetFallibleMalloc(const size_t FailAt);
void *FallibleMalloc(const size_t Size);

// Realloc.
void  ResetFallibleRealloc(void);
void  SetFallibleRealloc(const size_t FailAt);
void *FallibleRealloc(void        *Ptr,
                      const size_t NewSize);

// AddOverflow.
void ResetFallibleAddOverflow(void);
void SetFallibleAddOverflow(const size_t FailAt);
bool FallibleAddOverflow(const size_t A,
                         const size_t B,
                         size_t      *Res);

// MulOverflow.
void ResetFallibleMulOverflow(void);
void SetFallibleMulOverflow(const size_t FailAt);
bool FallibleMulOverflow(const size_t A,
                         const size_t B,
                         size_t      *Res);

#endif
