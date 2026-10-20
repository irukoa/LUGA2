#ifndef PriorityQueues_H
#define PriorityQueues_H

#include <stdbool.h>
#include <stddef.h>

typedef struct IndexedMinBinaryHeap IndexedMinBinaryHeap;

void FreeIndexedMinBinaryHeap(IndexedMinBinaryHeap **HeapInstanceReference);

IndexedMinBinaryHeap *NewIndexedMinBinaryHeap(const size_t Capacity);

void IndexedMinBinaryHeapInitialize(IndexedMinBinaryHeap *HeapInstance);

bool IndexedMinBinaryHeapIsEmpty(const IndexedMinBinaryHeap *HeapInstance);

bool IndexedMinBinaryHeapContains(const IndexedMinBinaryHeap *HeapInstance,
                                  const size_t                Element);

bool IndexedMinBinaryHeapDecreaseKey(IndexedMinBinaryHeap *HeapInstance,
                                     const size_t          Element,
                                     const double          NewPriority);

bool IndexedMinBinaryHeapExtractMinimum(IndexedMinBinaryHeap *HeapInstance,
                                        size_t               *Element,
                                        double               *Priority);

bool IndexedMinBinaryHeapPeekMinimum(IndexedMinBinaryHeap *HeapInstance,
                                     size_t               *Element,
                                     double               *Priority);

#endif
