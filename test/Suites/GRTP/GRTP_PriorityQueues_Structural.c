#include "src/Core/Core.h"
#include "src/GRTP/PriorityQueues.h"
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

TEST(FreeIndexedMinBinaryHeap_BadInput) { FreeIndexedMinBinaryHeap(NULL); }

TEST(FreeIndexedMinBinaryHeap_Nominal) {
  IndexedMinBinaryHeap *Heap = NULL;

  Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(Heap);
  FreeIndexedMinBinaryHeap(&Heap);
  ASSERT(!Heap);
}

TEST(NewIndexedMinBinaryHeap_BadInput) {
  IndexedMinBinaryHeap *Heap = NULL;

  Heap = NewIndexedMinBinaryHeap(0);
  ASSERT(!Heap);
}

TEST_F(StructuralCtx,
       NewIndexedMinBinaryHeap_MallocFail1) {
  (void)ContextData;
  IndexedMinBinaryHeap *Heap = NULL;

  SetFallibleMalloc(1);
  Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(!Heap);
}

TEST_F(StructuralCtx,
       NewIndexedMinBinaryHeap_MallocFail2) {
  (void)ContextData;
  IndexedMinBinaryHeap *Heap = NULL;

  SetFallibleMalloc(2);
  Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(!Heap);
}

TEST_F(StructuralCtx,
       NewIndexedMinBinaryHeap_MallocFail3) {
  (void)ContextData;
  IndexedMinBinaryHeap *Heap = NULL;

  SetFallibleMalloc(3);
  Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(!Heap);
}

TEST_F(StructuralCtx,
       NewIndexedMinBinaryHeap_MallocFail4) {
  (void)ContextData;
  IndexedMinBinaryHeap *Heap = NULL;

  SetFallibleMalloc(4);
  Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(!Heap);
}

TEST_F(StructuralCtx,
       NewIndexedMinBinaryHeap_HeapSizeMulOverflowFail) {
  (void)ContextData;
  IndexedMinBinaryHeap *Heap = NULL;

  SetFallibleMulOverflow(1);
  Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(!Heap);
}

TEST_F(StructuralCtx,
       NewIndexedMinBinaryHeap_PrioritySizeMulOverflowFail) {
  (void)ContextData;
  IndexedMinBinaryHeap *Heap = NULL;

  SetFallibleMulOverflow(2);
  Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(!Heap);
}

TEST(IndexedMinBinaryHeapInitialize_BadInput) {
  IndexedMinBinaryHeapInitialize(NULL);
}

TEST(IndexedMinBinaryHeapIsEmpty_BadInput) {
  ASSERT(IndexedMinBinaryHeapIsEmpty(NULL));
}

TEST(IndexedMinBinaryHeapContains_BadInput) {
  ASSERT(!IndexedMinBinaryHeapContains(NULL, 0));
}

TEST(IndexedMinBinaryHeapDecreaseKey_BadInput) {
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(NULL, 0, (double)0.0));
}

TEST(IndexedMinBinaryHeapExtractMinimum_BadInput) {
  ASSERT(!IndexedMinBinaryHeapExtractMinimum(NULL, NULL, NULL));
}

TEST(IndexedMinBinaryHeapPeekMinimum_BadInput) {
  ASSERT(!IndexedMinBinaryHeapPeekMinimum(NULL, NULL, NULL));
}

TEST_SUITE(IndexedMinBinaryHeapMemorySuite,
           FreeIndexedMinBinaryHeap_BadInput,
           FreeIndexedMinBinaryHeap_Nominal,
           NewIndexedMinBinaryHeap_BadInput,
           NewIndexedMinBinaryHeap_MallocFail1,
           NewIndexedMinBinaryHeap_MallocFail2,
           NewIndexedMinBinaryHeap_MallocFail3,
           NewIndexedMinBinaryHeap_MallocFail4,
           NewIndexedMinBinaryHeap_HeapSizeMulOverflowFail,
           NewIndexedMinBinaryHeap_PrioritySizeMulOverflowFail,
           IndexedMinBinaryHeapInitialize_BadInput,
           IndexedMinBinaryHeapIsEmpty_BadInput,
           IndexedMinBinaryHeapContains_BadInput,
           IndexedMinBinaryHeapDecreaseKey_BadInput,
           IndexedMinBinaryHeapExtractMinimum_BadInput,
           IndexedMinBinaryHeapPeekMinimum_BadInput);
