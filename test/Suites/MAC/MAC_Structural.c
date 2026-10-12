#include "src/Core/Core.h"
#include "src/MAC/MultidimensionalArrayContainers.h"
#include "test/CoreSupport/CoreSupport.h"
#include "test/TSD/TSD.h"

CONTEXT_FN(StructuralSetup) {
  (void)ContextData;
  Core_Malloc = FallibleMalloc;
  ResetFallibleMalloc();
  Core_MulOverflow = FallibleMulOverflow;
  ResetFallibleMulOverflow();
}
CONTEXT_FN(StructuralTeardown) {
  (void)ContextData;
  RESET_FN(Core_Malloc);
  RESET_FN(Core_MulOverflow);
}
CONTEXT(StructuralCtx,
        StructuralSetup,
        StructuralTeardown,
        (size_t)0);

TEST(FreeMAC_BadInput) { FreeMAC(NULL); }

TEST(FreeMAC_Nominal) {
  MAC *Container = NULL;

  Container = NewMAC(1, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(Container);
  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST(NewMAC_BadInput) {
  MAC *Container = NULL;

  Container = NewMAC(0, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(!Container);
  Container = NewMAC(1, NULL, COLUMN_MAJOR);
  ASSERT(!Container);
  Container = NewMAC(1, (size_t[]){1}, ROW_MAJOR + 1);
  ASSERT(!Container);
}

TEST_F(StructuralCtx,
       NewMAC_OverflowFail) {
  (void)ContextData;
  MAC *Container = NULL;

  SetFallibleMulOverflow(1);
  Container = NewMAC(1, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(!Container);
}

TEST_F(StructuralCtx,
       NewMAC_ShapeSizeMulOverflowFail) {
  (void)ContextData;
  MAC *Container = NULL;

  SetFallibleMulOverflow(2);
  Container = NewMAC(1, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(!Container);
}

TEST_F(StructuralCtx,
       NewMAC_MainAllocFail) {
  (void)ContextData;
  MAC *Container = NULL;

  SetFallibleMalloc(1);
  Container = NewMAC(1, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(!Container);
}

TEST_F(StructuralCtx,
       NewMAC_ShapeAllocFail) {
  (void)ContextData;
  MAC *Container = NULL;

  SetFallibleMalloc(2);
  Container = NewMAC(1, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(!Container);
}

TEST(NewMAC_Nominal) {
  MAC          *Container = NULL;
  const size_t *ShapePrt;

  Container = NewMAC(3, (size_t[]){1, 2, 3}, COLUMN_MAJOR);
  ASSERT(Container);

  ASSERT(MACGetRank(Container) == 3);
  ShapePrt = MACGetShape(Container);
  ASSERT(ShapePrt[0] == 1);
  ASSERT(ShapePrt[1] == 2);
  ASSERT(ShapePrt[2] == 3);
  ASSERT(MACGetElementCount(Container) == 1 * 2 * 3);
  ASSERT(MACGetLayout(Container) == COLUMN_MAJOR);

  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST_SUITE(MACStructuralMemorySuite,
           FreeMAC_BadInput,
           FreeMAC_Nominal,
           NewMAC_BadInput,
           NewMAC_OverflowFail,
           NewMAC_ShapeSizeMulOverflowFail,
           NewMAC_MainAllocFail,
           NewMAC_ShapeAllocFail,
           NewMAC_Nominal);

TEST(MACNullFunctionReturn_Nominal) {
  ASSERT(MACGetRank(NULL) == 0);
  ASSERT(MACGetShape(NULL) == NULL);
  ASSERT(MACGetElementCount(NULL) == 0);
  ASSERT(MACGetLayout(NULL) == 0);
}

TEST(MACFlatToMultidimensionalReturn_Nominal) {
  MAC   *Container                = NULL;
  size_t MultidimensionalIndex[2] = {0x12345678};

  Container = NewMAC(1, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(Container);

  MACFlatToMultidimensional(NULL, 1, 0, MultidimensionalIndex);
  ASSERT(MultidimensionalIndex[0] == 0x12345678);

  MACFlatToMultidimensional(Container, 2, 0, MultidimensionalIndex);
  ASSERT(MultidimensionalIndex[0] == 0x12345678);

  MACFlatToMultidimensional(Container, 1, 1, MultidimensionalIndex);
  ASSERT(MultidimensionalIndex[0] == 0x12345678);

  MACFlatToMultidimensional(Container, 1, 0, NULL);
  ASSERT(MultidimensionalIndex[0] == 0x12345678);

  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST(MACMultidimensionalToFlatReturn_Nominal) {
  MAC   *Container = NULL;
  size_t FlatIndex = 0x12345678;

  Container = NewMAC(1, (size_t[]){1}, COLUMN_MAJOR);
  ASSERT(Container);

  MACMultidimensionalToFlat(NULL, 1, (size_t[]){0}, &FlatIndex);
  ASSERT(FlatIndex == 0x12345678);

  MACMultidimensionalToFlat(Container, 2, (size_t[]){0, 0}, &FlatIndex);
  ASSERT(FlatIndex == 0x12345678);

  MACMultidimensionalToFlat(Container, 1, NULL, &FlatIndex);
  ASSERT(FlatIndex == 0x12345678);

  MACMultidimensionalToFlat(Container, 1, (size_t[]){1}, &FlatIndex);
  ASSERT(FlatIndex == 0x12345678);

  MACMultidimensionalToFlat(Container, 1, (size_t[]){0}, NULL);
  ASSERT(FlatIndex == 0x12345678);

  FreeMAC(&Container);
  ASSERT(!Container);
}

TEST_SUITE(MACStructuralFunctionReturnSuite,
           MACNullFunctionReturn_Nominal,
           MACFlatToMultidimensionalReturn_Nominal,
           MACMultidimensionalToFlatReturn_Nominal);
