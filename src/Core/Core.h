#ifndef LUGA_Core_H
#define LUGA_Core_H

#include "src/Core/FunctionMacros.h"
#include <stdbool.h>
#include <stdlib.h>

APIDEC(void *,
       Core_Malloc,
       const size_t Size);

APIDEC(void *,
       Core_Realloc,
       void        *Ptr,
       const size_t NewSize);

APIDEC(bool,
       Core_AddOverflow,
       const size_t A,
       const size_t B,
       size_t      *Res);

APIDEC(bool,
       Core_MulOverflow,
       const size_t A,
       const size_t B,
       size_t      *Res);

#endif
