#include "src/GRTP/GraphUtilities.h"
#include "src/GRTP/PriorityQueues.h"
#include "test/TSD/TSD.h"
#include <float.h>
#include <math.h>

typedef struct {
  size_t  Capacity;
  size_t  ActualSize;
  size_t *Heap;
  size_t *Position;
  double *Priority;
} IndexedMinBinaryHeapClone;

TEST(GRTPIndexedMinBinaryHeap_New) {
  IndexedMinBinaryHeap *Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(Heap);
  IndexedMinBinaryHeapClone *Clone = (IndexedMinBinaryHeapClone *)Heap;

  IndexedMinBinaryHeapInitialize(Heap);
  ASSERT(Clone->Capacity == 1);
  ASSERT(Clone->ActualSize == 1);
  ASSERT(Clone->Heap[0] == 0);
  ASSERT(Clone->Position[0] == 0);
  ASSERT(isinf(Clone->Priority[0]));

  FreeIndexedMinBinaryHeap(&Heap);
  ASSERT(!Heap);
}

TEST(GRTPIndexedMinBinaryHeap_Functional) {
  size_t                Minimum;
  double                Priority;
  IndexedMinBinaryHeap *Heap = NewIndexedMinBinaryHeap(10);
  ASSERT(Heap);
  IndexedMinBinaryHeapClone *Clone = (IndexedMinBinaryHeapClone *)Heap;

  IndexedMinBinaryHeapInitialize(Heap);
  ASSERT(Clone->Capacity == 10);
  ASSERT(Clone->ActualSize == 10);

  ASSERT(!IndexedMinBinaryHeapIsEmpty(Heap));

  // Decrease smallest.
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 0, (double)1.0));
  ASSERT(IndexedMinBinaryHeapPeekMinimum(Heap, NULL, NULL));

  // Decrease other.
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 7, (double)1.0));
  ASSERT(IndexedMinBinaryHeapPeekMinimum(Heap, &Minimum, &Priority));
  ASSERT(Minimum == 0);
  ASSERT(fabs(Priority - (double)1.0) < 10 * DBL_EPSILON);

  // Extract.
  ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, NULL, NULL));
  ASSERT(Clone->ActualSize == 9);
  ASSERT(!IndexedMinBinaryHeapContains(Heap, 0));
  ASSERT(IndexedMinBinaryHeapContains(Heap, 9));
  ASSERT(!IndexedMinBinaryHeapContains(Heap, 10));

  // Decrease other, make minimum, and extract.
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 5, (double)0.0));
  ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, &Minimum, &Priority));
  ASSERT(Clone->ActualSize == 8);
  ASSERT(Minimum == 5);
  ASSERT(fabs(Priority) < 10 * DBL_EPSILON);
  ASSERT(!IndexedMinBinaryHeapContains(Heap, 5));
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(Heap, 5, (double)10.0));
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(Heap, 5, (double)-10.0));

  // Decrease inexistent.
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(Heap, 10, (double)0.0));

  // Decrease other four.
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 1, (double)7.0));
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 9, (double)4.0));
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 6, (double)9.0));
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 3, (double)2.0));

  // Extract the four smallest.
  ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, &Minimum, &Priority));
  ASSERT(Minimum == 7);
  ASSERT(fabs(Priority - (double)1.0) < 10 * DBL_EPSILON);
  ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, &Minimum, &Priority));
  ASSERT(Minimum == 3);
  ASSERT(fabs(Priority - (double)2.0) < 10 * DBL_EPSILON);
  ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, &Minimum, &Priority));
  ASSERT(Minimum == 9);
  ASSERT(fabs(Priority - (double)4.0) < 10 * DBL_EPSILON);
  ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, &Minimum, &Priority));
  ASSERT(Minimum == 1);
  ASSERT(fabs(Priority - (double)7.0) < 10 * DBL_EPSILON);

  ASSERT(Clone->ActualSize == 4);

  // Extract until empty.
  while (!IndexedMinBinaryHeapIsEmpty(Heap)) {
    ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, NULL, NULL));
  }
  ASSERT(IndexedMinBinaryHeapIsEmpty(Heap));
  ASSERT(!IndexedMinBinaryHeapExtractMinimum(Heap, NULL, NULL));
  ASSERT(!IndexedMinBinaryHeapPeekMinimum(Heap, NULL, NULL));

  FreeIndexedMinBinaryHeap(&Heap);
  ASSERT(!Heap);
}

TEST(GRTPIndexedMinBinaryHeap_DecreaseKeyRejectsNonLowering) {
  size_t                Minimum;
  double                Priority;
  IndexedMinBinaryHeap *Heap = NewIndexedMinBinaryHeap(1);
  ASSERT(Heap);
  IndexedMinBinaryHeapClone *Clone = (IndexedMinBinaryHeapClone *)Heap;

  IndexedMinBinaryHeapInitialize(Heap);

  // Establish a finite stored priority.
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 0, (double)1.0));
  ASSERT(fabs(Clone->Priority[0] - (double)1.0) < 10 * DBL_EPSILON);

  // DecreaseKey must reject a NewPriority equal to the stored priority while
  // the element remains in the heap.
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(Heap, 0, (double)1.0));

  // DecreaseKey must reject a NewPriority greater than the stored priority
  // while the element remains in the heap.
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(Heap, 0, (double)3.0));

  // The rejected calls must not modify the heap state.
  ASSERT(Clone->ActualSize == 1);
  ASSERT(Clone->Heap[0] == 0);
  ASSERT(Clone->Position[0] == 0);
  ASSERT(fabs(Clone->Priority[0] - (double)1.0) < 10 * DBL_EPSILON);

  // A strictly lower priority must still succeed.
  ASSERT(IndexedMinBinaryHeapDecreaseKey(Heap, 0, (double)0.5));
  ASSERT(IndexedMinBinaryHeapPeekMinimum(Heap, &Minimum, &Priority));
  ASSERT(Minimum == 0);
  ASSERT(fabs(Priority - (double)0.5) < 10 * DBL_EPSILON);

  // The initial priority of an uninitialized heap element is INFINITY, so a
  // finite NewPriority must be accepted (strictly lower than INFINITY).
  // After extraction, the same element cannot be decreased again regardless
  // of the requested priority.
  ASSERT(IndexedMinBinaryHeapExtractMinimum(Heap, NULL, NULL));
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(Heap, 0, (double)0.25));
  ASSERT(!IndexedMinBinaryHeapDecreaseKey(Heap, 0, (double)10.0));

  FreeIndexedMinBinaryHeap(&Heap);
  ASSERT(!Heap);
}

TEST_SUITE(GRTPIndexedMinBinaryHeap,
           GRTPIndexedMinBinaryHeap_New,
           GRTPIndexedMinBinaryHeap_Functional,
           GRTPIndexedMinBinaryHeap_DecreaseKeyRejectsNonLowering);
