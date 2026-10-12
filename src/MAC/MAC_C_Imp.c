#include "src/Core/Core.h"
#include "src/MAC/MultidimensionalArrayContainers.h"

struct MAC {
  size_t    Rank;
  size_t   *Shape;
  size_t    ElementCount;
  MACLayout Layout;
};

void FreeMAC(MAC **ContainerReference) {
  if (!ContainerReference) {
    return;
  }
  if (*ContainerReference) {
    free((*ContainerReference)->Shape);
    free(*ContainerReference);
    *ContainerReference = NULL;
  }
}

MAC *NewMAC(const size_t    Rank,
            const size_t    Shape[Rank],
            const MACLayout Layout) {
  MAC   *Container = NULL;
  size_t i;
  size_t ShapeSize;

  size_t ElementCount = (size_t)1;
  size_t Reference;

  if (Rank == 0) {
    return NULL;
  }
  if (!Shape) {
    return NULL;
  }
  if ((Layout != COLUMN_MAJOR) && (Layout != ROW_MAJOR)) {
    return NULL;
  }
  for (i = 0; i < Rank; i++) {
    Reference = ElementCount;
    // Note: Shape[i] = 0 allowed.
    if (Core_MulOverflow(Reference, Shape[i], &ElementCount)) {
      return NULL;
    }
  }
  Container = (MAC *)Core_Malloc(sizeof(MAC));
  if (!Container) {
    return NULL;
  }
  Container->Shape = NULL;
  if (Core_MulOverflow(Rank, sizeof(size_t), &ShapeSize)) {
    free(Container);
    return NULL;
  }
  Container->Shape = (size_t *)Core_Malloc(ShapeSize);
  if (!Container->Shape) {
    free(Container);
    return NULL;
  }
  Container->Rank = Rank;
  for (i = 0; i < Rank; i++) {
    Container->Shape[i] = Shape[i];
  }
  Container->ElementCount = ElementCount;
  Container->Layout       = Layout;
  return Container;
}

size_t MACGetRank(const MAC *Container) {
  if (Container) {
    return Container->Rank;
  }
  return (size_t)0;
}

const size_t *MACGetShape(const MAC *Container) {
  if (Container) {
    return Container->Shape;
  }
  return (size_t *)NULL;
}

size_t MACGetElementCount(const MAC *Container) {
  if (Container) {
    return Container->ElementCount;
  }
  return (size_t)0;
}

MACLayout MACGetLayout(const MAC *Container) {
  if (Container) {
    return Container->Layout;
  }
  return (MACLayout)0;
}

void MACFlatToMultidimensional(const MAC   *Container,
                               const size_t Rank,
                               const size_t FlatIndex,
                               size_t       MultidimensionalIndex[Rank]) {
  size_t i;
  size_t Counter;
  if (!Container) {
    return;
  }
  if (Container->Rank != Rank) {
    return;
  }
  if (FlatIndex >= Container->ElementCount) {
    return;
  }
  if (!MultidimensionalIndex) {
    return;
  }
  Counter = FlatIndex;
  switch (Container->Layout) {
  case COLUMN_MAJOR:
    for (i = 0; i < Container->Rank; i++) {
      MultidimensionalIndex[i] = Counter % Container->Shape[i];
      Counter = (Counter - MultidimensionalIndex[i]) / Container->Shape[i];
    }
    break;
  case ROW_MAJOR:
    for (i = Container->Rank; i-- > 0;) {
      MultidimensionalIndex[i] = Counter % Container->Shape[i];
      Counter = (Counter - MultidimensionalIndex[i]) / Container->Shape[i];
    }
    break;
  }
}

void MACMultidimensionalToFlat(const MAC   *Container,
                               const size_t Rank,
                               const size_t MultidimensionalIndex[Rank],
                               size_t      *FlatIndex) {
  size_t i;
  size_t Counter;
  if (!Container) {
    return;
  }
  if (Container->Rank != Rank) {
    return;
  }
  if (!MultidimensionalIndex) {
    return;
  }
  for (i = 0; i < Rank; i++) {
    if (MultidimensionalIndex[i] >= Container->Shape[i]) {
      return;
    }
  }
  if (!FlatIndex) {
    return;
  }
  Counter = 0;
  switch (Container->Layout) {
  case COLUMN_MAJOR:
    for (i = Container->Rank; i-- > 0;) {
      Counter = Counter * Container->Shape[i] + MultidimensionalIndex[i];
    }
    break;
  case ROW_MAJOR:
    for (i = 0; i < Container->Rank; i++) {
      Counter = Counter * Container->Shape[i] + MultidimensionalIndex[i];
    }
    break;
  }
  *FlatIndex = Counter;
}
