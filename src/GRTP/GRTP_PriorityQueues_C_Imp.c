#include "src/Core/Core.h"
#include "src/GRTP/GraphUtilities.h"
#include "src/GRTP/PriorityQueues.h"
#include <stdint.h>

struct IndexedMinBinaryHeap {
  size_t  Capacity;
  size_t  ActualSize;
  size_t *Heap;
  size_t *Position;
  double *Priority;
};

static void IndexedMinBinaryHeapSwap(IndexedMinBinaryHeap *HeapInstance,
                                     size_t                A,
                                     size_t                B) {
  size_t Tmp;

  Tmp                                           = HeapInstance->Heap[A];
  HeapInstance->Heap[A]                         = HeapInstance->Heap[B];
  HeapInstance->Heap[B]                         = Tmp;
  HeapInstance->Position[HeapInstance->Heap[A]] = A;
  HeapInstance->Position[HeapInstance->Heap[B]] = B;
}

static void IndexedMinBinaryHeapBubbleUp(IndexedMinBinaryHeap *HeapInstance,
                                         size_t                Element) {
  size_t Parent;

  while (Element > 0) {
    Parent = (Element - 1) / 2;
    if (HeapInstance->Priority[HeapInstance->Heap[Parent]] <=
        HeapInstance->Priority[HeapInstance->Heap[Element]]) {
      break;
    }
    IndexedMinBinaryHeapSwap(HeapInstance, Parent, Element);
    Element = Parent;
  }
}

static void IndexedMinBinaryHeapBubbleDown(IndexedMinBinaryHeap *HeapInstance,
                                           size_t                Element) {
  size_t Left;
  size_t Right;
  size_t Smallest;

  for (;;) {
    Left     = 2 * Element + 1;
    Right    = Left + 1;
    Smallest = Element;
    if ((Left < HeapInstance->ActualSize) &&
        (HeapInstance->Priority[HeapInstance->Heap[Left]] <
         HeapInstance->Priority[HeapInstance->Heap[Smallest]])) {
      Smallest = Left;
    }
    if ((Right < HeapInstance->ActualSize) &&
        (HeapInstance->Priority[HeapInstance->Heap[Right]] <
         HeapInstance->Priority[HeapInstance->Heap[Smallest]])) {
      Smallest = Right;
    }
    if (Smallest == Element) {
      break;
    }
    IndexedMinBinaryHeapSwap(HeapInstance, Element, Smallest);
    Element = Smallest;
  }
}

void FreeIndexedMinBinaryHeap(IndexedMinBinaryHeap **HeapInstanceReference) {
  if (!HeapInstanceReference) {
    return;
  }
  if (*HeapInstanceReference) {
    free((*HeapInstanceReference)->Priority);
    free((*HeapInstanceReference)->Position);
    free((*HeapInstanceReference)->Heap);
    free((*HeapInstanceReference));
    *HeapInstanceReference = NULL;
  }
}

IndexedMinBinaryHeap *NewIndexedMinBinaryHeap(const size_t Capacity) {
  IndexedMinBinaryHeap *HeapInstance = NULL;
  size_t                Size;
  bool                  AllocatedHeapInstance = false;
  bool                  AllocatedHeap         = false;
  bool                  AllocatedPosition     = false;

  if (Capacity == 0) {
    return NULL;
  }
  HeapInstance =
      (IndexedMinBinaryHeap *)Core_Malloc(sizeof(IndexedMinBinaryHeap));
  if (!HeapInstance) {
    goto cleanup;
  }
  AllocatedHeapInstance = true;
  if (Core_MulOverflow(Capacity, sizeof(size_t), &Size)) {
    goto cleanup;
  }
  HeapInstance->Heap = (size_t *)Core_Malloc(Size);
  if (!HeapInstance->Heap) {
    goto cleanup;
  }
  AllocatedHeap          = true;
  HeapInstance->Position = (size_t *)Core_Malloc(Size);
  if (!HeapInstance->Position) {
    goto cleanup;
  }
  AllocatedPosition = true;
  if (Core_MulOverflow(Capacity, sizeof(double), &Size)) {
    goto cleanup;
  }
  HeapInstance->Priority = (double *)Core_Malloc(Size);
  if (!HeapInstance->Priority) {
    goto cleanup;
  }
  HeapInstance->Capacity   = Capacity;
  HeapInstance->ActualSize = Capacity;
  return HeapInstance;
cleanup:
  if (AllocatedPosition)
    free(HeapInstance->Position);
  if (AllocatedHeap)
    free(HeapInstance->Heap);
  if (AllocatedHeapInstance)
    free(HeapInstance);
  return NULL;
}

void IndexedMinBinaryHeapInitialize(IndexedMinBinaryHeap *HeapInstance) {
  size_t i;

  if (!HeapInstance) {
    return;
  }
  HeapInstance->ActualSize = HeapInstance->Capacity;
  for (i = 0; i < HeapInstance->Capacity; i++) {
    HeapInstance->Heap[i]     = i;
    HeapInstance->Position[i] = i;
    HeapInstance->Priority[i] = UnreachableDistance;
  }
}

bool IndexedMinBinaryHeapIsEmpty(const IndexedMinBinaryHeap *HeapInstance) {
  if (!HeapInstance) {
    return true;
  }
  if (HeapInstance->ActualSize == 0) {
    return true;
  }
  return false;
}

bool IndexedMinBinaryHeapContains(const IndexedMinBinaryHeap *HeapInstance,
                                  const size_t                Element) {
  if (!HeapInstance) {
    return false;
  }
  if (Element >= HeapInstance->Capacity) {
    return false;
  }
  return (bool)(HeapInstance->Position[Element] != UnreachableVertex);
}

bool IndexedMinBinaryHeapDecreaseKey(IndexedMinBinaryHeap *HeapInstance,
                                     const size_t          Element,
                                     const double          NewPriority) {
  if (!HeapInstance) {
    return false;
  }
  if (Element >= HeapInstance->Capacity) {
    return false;
  }
  if (NewPriority >= HeapInstance->Priority[Element]) {
    return false;
  }
  if (HeapInstance->Position[Element] == UnreachableVertex) {
    return false;
  }
  HeapInstance->Priority[Element] = NewPriority;
  IndexedMinBinaryHeapBubbleUp(HeapInstance, HeapInstance->Position[Element]);
  return true;
}

bool IndexedMinBinaryHeapExtractMinimum(IndexedMinBinaryHeap *HeapInstance,
                                        size_t               *Element,
                                        double               *Priority) {
  size_t Minimum;

  if (!HeapInstance) {
    return false;
  }
  if (HeapInstance->ActualSize == 0) {
    return false;
  }
  Minimum = HeapInstance->Heap[0];
  IndexedMinBinaryHeapSwap(HeapInstance, 0, HeapInstance->ActualSize - 1);
  HeapInstance->ActualSize--;
  if (HeapInstance->ActualSize > 0) {
    IndexedMinBinaryHeapBubbleDown(HeapInstance, 0);
  }
  HeapInstance->Position[Minimum] = UnreachableVertex;
  if (Element) {
    *Element = Minimum;
  }
  if (Priority) {
    *Priority = HeapInstance->Priority[Minimum];
  }
  return true;
}

bool IndexedMinBinaryHeapPeekMinimum(IndexedMinBinaryHeap *HeapInstance,
                                     size_t               *Element,
                                     double               *Priority) {
  size_t Minimum;

  if (!HeapInstance) {
    return false;
  }
  if (HeapInstance->ActualSize == 0) {
    return false;
  }
  Minimum = HeapInstance->Heap[0];
  if (Element) {
    *Element = Minimum;
  }
  if (Priority) {
    *Priority = HeapInstance->Priority[Minimum];
  }
  return true;
}
